#include "mainwindow.h"

#include "colmap/scene/database.h"

#include <QFileDialog>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <Eigen/Dense>

void MainWindow::ApplyScene() {
  const auto& images = scene.Images();
  // const auto& image_to_features = scene.ImageToFeatures();
  // const auto& image1_image2_matches = scene.Image1Image2Matches();
  this->image_list->clear();
  this->image_match_list->clear();
  this->image_list->setRowCount(images.size());
  int row = 0;
  for (int i = 0; i < images.size(); i++) {
    QTableWidgetItem* id = new QTableWidgetItem(tr("%1").arg(i));
    this->image_list->setItem(row, 0, id);
    QTableWidgetItem* file =
        new QTableWidgetItem(QFileInfo(images[i].image_file.c_str())
                                 .fileName()
                                 .toStdString()
                                 .c_str());
    this->image_list->setItem(row, 1, file);
    row++;
  }
  this->image_list->resizeColumnsToContents();
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setupUi(this);
  this->image_holder->set_scene(&scene);
  connect(this->image_list,
          &QTableWidget::currentCellChanged,
          this,
          &MainWindow::WhenImageSelected);
  connect(this->image_match_list,
          &QTableWidget::currentCellChanged,
          this,
          &MainWindow::WhenMatchedImageSelected);
}

void MainWindow::on_actionOpen_COLMAP_project_triggered() {
  static QString last_database_file_name = "";
  const QString file_name = QFileDialog::getOpenFileName(
      this, tr("Open COLMAP database file"), last_database_file_name);
  if (file_name.isNull()) return;
  last_database_file_name = file_name;

  static QString last_image_dir_name = "";
  const QString image_dir =
      QFileDialog::getExistingDirectory(this,
                                        tr("Open image directory"),
                                        QFileInfo(file_name).dir().path(),
                                        QFileDialog::ShowDirsOnly);
  if (image_dir.isNull()) return;
  last_image_dir_name = image_dir;

  colmap::Database database(file_name.toStdString());
  std::filesystem::path image_path(image_dir.toStdString());

  const std::vector<colmap::Camera> cameras = database.ReadAllCameras();
  std::map<colmap::camera_t, const colmap::Camera*> camera_map;
  for (const auto& cam : cameras) {
    camera_map[cam.camera_id] = &cam;
  }
  const std::vector<colmap::Image> images = database.ReadAllImages();

  std::vector<ImageInfo> image_infos;

  std::vector<std::vector<AbstractFeature>> image_to_features;
  std::vector<std::map<int, std::vector<std::pair<int, int>>>>
      image1_image2_matches;

  for (int i = 0; i < images.size(); i++) {
    const auto& im = images.at(i);
    ImageInfo info;
    std::string img_name;
    info.image_file = (image_path / im.Name()).string();

    const auto& cam = *(camera_map.at(im.CameraId()));

    info.width = cam.width;
    info.height = cam.height;

    image_infos.emplace_back(info);
  }

  for (int idx = 0; idx < image_infos.size(); idx++) {
    const auto& im = images.at(idx);
    const colmap::FeatureKeypoints kpts = database.ReadKeypoints(im.ImageId());
    std::vector<AbstractFeature>& features = image_to_features.emplace_back();
    for (int k = 0; k < kpts.size(); k++) {
      const auto& kpt = kpts[k];

      Eigen::Matrix2f A;
      A << kpt.a11, kpt.a12, kpt.a21, kpt.a22;

      const Eigen::Vector2f v(kpt.x, kpt.y);
      const Eigen::Vector2f dir(1.0, 0.0);
      const Eigen::Vector2f rotated = A * dir;
      const Eigen::Vector2f unit_dir = rotated.normalized();
      const double scale = std::sqrt(A.determinant());
      const double angle = std::atan2(unit_dir.y(), unit_dir.x());

      AbstractFeature f;
      f.pos = QPointF(kpt.x, kpt.y);
      f.scale = scale;
      f.orient = angle;
      features.push_back(f);
    }
  }

  for (int im1 = 0; im1 < image_infos.size(); im1++) {
    image1_image2_matches.emplace_back();
    auto& im1_imn_matches = image1_image2_matches.back();
    for (int im2 = 0; im2 < image_infos.size(); im2++) {
      if (im1 == im2) continue;

      const auto a = images[im1].ImageId();
      const auto b = images[im2].ImageId();

      if (!database.ExistsMatches(a, b)) {
        continue;
      }
      const std::vector<colmap::FeatureMatch> matches =
          database.ReadMatches(a, b);

      const colmap::FeatureKeypoints kpts_a = database.ReadKeypoints(a);
      const colmap::FeatureKeypoints kpts_b = database.ReadKeypoints(b);

      auto& im1_im2_matches = im1_imn_matches[im2];
      for (const auto& match : matches) {
        im1_im2_matches.emplace_back(match.point2D_idx1, match.point2D_idx2);
      }
    }
  }

  scene.setScene(image_infos, image_to_features, image1_image2_matches);
  ApplyScene();
}

void MainWindow::WhenImageSelected(int r, int c) {
  const auto item = this->image_list->item(r, 0);
  bool ok;
  const int img_id = item->text().toInt(&ok);
  if (!ok) return;

  const auto& images = scene.Images();
  const auto& image_matches = scene.Image1Image2Matches()[img_id];
  this->image_match_list->clear();
  this->image_match_list->setRowCount(image_matches.size());
  int row = 0;
  for (const auto& it : image_matches) {
    QTableWidgetItem* id2 = new QTableWidgetItem(tr("%1").arg(it.first));
    this->image_match_list->setItem(row, 0, id2);
    QTableWidgetItem* file =
        new QTableWidgetItem(QFileInfo(images.at(it.first).image_file.c_str())
                                 .fileName()
                                 .toStdString()
                                 .c_str());
    this->image_match_list->setItem(row, 1, file);
    QTableWidgetItem* count =
        new QTableWidgetItem(tr("%1").arg(it.second.size()));
    this->image_match_list->setItem(row, 2, count);
    row++;
  }
  this->image_match_list->resizeColumnsToContents();

  this->image_holder->LoadImageLeft(img_id);
  this->image_holder->updateForce();
}

void MainWindow::WhenMatchedImageSelected(int r, int c) {
  if (r < 0 || c < 0) return;
  const int row1 = this->image_list->currentRow();
  const auto item1 = this->image_list->item(row1, 0);
  const auto item2 = this->image_match_list->item(r, 0);
  bool ok;
  const int img2_id = item2->text().toInt(&ok);
  if (!ok) return;
  const int img1_id = item1->text().toInt(&ok);
  if (!ok) return;

  const std::vector<std::pair<int, int>>& image_matches =
      scene.Image1Image2Matches()[img1_id].at(img2_id);

  this->image_holder->LoadImageRight(img2_id);
  std::map<int, int> matches;
  for (const auto it : image_matches) {
    matches[it.first] = it.second;
  }
  this->image_holder->SetMatches(matches);
  this->image_holder->updateForce();
}

void MainWindow::on_size_slider_valueChanged(int size) {
  this->image_holder->SetMaxFeatureSize(
      std::pow(10.0, double(size) / 100. * 5.0));
  this->image_holder->updateForce();
}

void MainWindow::on_button_hide_feature_marker_toggled(bool checked) {
  this->image_holder->SetShowFeatures(!checked);
  this->image_holder->updateForce();
}

void MainWindow::on_button_show_only_matched_toggled(bool checked) {
  this->image_holder->SetOnlyShowMatchedFeatures(checked);
  this->image_holder->updateForce();
}

void MainWindow::on_button_show_match_line_toggled(bool checked) {
  this->image_holder->SetShowMatchLines(checked);
  this->image_holder->updateForce();
}

void MainWindow::on_button_show_epipolar_toggled(bool checked) {
  this->image_holder->SetShowEpipolar(checked);
  this->image_holder->updateForce();
}
