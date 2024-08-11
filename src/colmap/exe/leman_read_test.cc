#include "colmap/exe/database.h"
#include "colmap/exe/feature.h"
#include "colmap/exe/image.h"
#include "colmap/exe/model.h"
#include "colmap/exe/mvs.h"
#include "colmap/exe/sfm.h"

#include <filesystem>
#include <fstream>

#include <Eigen/Dense>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

DEFINE_string(database, "", "data base file path");
DEFINE_string(image_path, "", "image folder path");
DEFINE_string(output_path, "", "output folder path");

using namespace colmap;

void DrawKeypoints(cv::Mat& im, const FeatureKeypoints& kpts) {
  for (const FeatureKeypoint& kpt : kpts) {
    Eigen::Matrix2f A;
    A << kpt.a11, kpt.a12, kpt.a21, kpt.a22;
    const Eigen::Vector2f v(kpt.x, kpt.y);
    const Eigen::Vector2f dir(0.0, 1.0);
    const Eigen::Vector2f rotated = A * dir;
    const Eigen::Vector2f p = v + rotated;
    const double scale = std::sqrt(A.determinant());
    cv::circle(im,
               cv::Point2f(kpt.x, kpt.y),
               std::max(2, (int)std::round(scale)),
               cv::Scalar(0, 255, 0),
               1,
               cv::LINE_8);
    cv::line(im,
             cv::Point2f(kpt.x, kpt.y),
             cv::Point2f(p.x(), p.y()),
             cv::Scalar(0, 0, 255),
             1,
             cv::LINE_8);
  }
}

void DumpMatches(colmap::Database& database,
                 const std::vector<Image>& images,
                 const std::filesystem::path& image_path,
                 const std::filesystem::path& output_path) {
  for (int i = 0; i < images.size(); i++) {
    for (int j = i + 1; j < images.size(); j++) {
      const auto a = images[i].ImageId();
      const auto b = images[j].ImageId();
      if (!database.ExistsMatches(a, b)) {
        continue;
      }
      const std::vector<FeatureMatch> matches = database.ReadMatches(a, b);
      LOG(INFO) << "Matches between " << images[i].ImageId() << " and "
                << images[j].ImageId() << ": " << matches.size();
      const FeatureKeypoints kpts_a = database.ReadKeypoints(a);
      const FeatureKeypoints kpts_b = database.ReadKeypoints(b);
      LOG(INFO) << "dumping matches";
      cv::Mat im_a = cv::imread((image_path / images[i].Name()).string(),
                                cv::IMREAD_UNCHANGED);
      cv::Mat im_b = cv::imread((image_path / images[j].Name()).string(),
                                cv::IMREAD_UNCHANGED);
      DrawKeypoints(im_a, kpts_a);
      DrawKeypoints(im_b, kpts_b);
      for (const auto& match : matches) {
        const auto& kpt_a = kpts_a[match.point2D_idx1];
        const auto& kpt_b = kpts_b[match.point2D_idx2];
        cv::line(im_a,
                 cv::Point2f(kpt_a.x, kpt_a.y),
                 cv::Point2f(kpt_b.x, kpt_b.y),
                 cv::Scalar(255, 0, 0),
                 1,
                 cv::LINE_8);
        cv::line(im_b,
                 cv::Point2f(kpt_a.x, kpt_a.y),
                 cv::Point2f(kpt_b.x, kpt_b.y),
                 cv::Scalar(255, 0, 0),
                 1,
                 cv::LINE_8);
      }
      const std::string pair_name = images[i].Name() + "_" + images[j].Name();
      cv::imwrite((output_path / (pair_name + "_a.png")).string(), im_a);
      cv::imwrite((output_path / (pair_name + "_b.png")).string(), im_b);
    }
  }
}

void SaveMatrixAsCSV(const Eigen::MatrixXi& matrix,
                     const std::string& filename) {
  std::ofstream file(filename);
  if (file.is_open()) {
    for (int i = 0; i < matrix.rows(); ++i) {
      for (int j = 0; j < matrix.cols(); ++j) {
        file << matrix(i, j);
        if (j < matrix.cols() - 1) {
          file << ",";  // Add a comma after each element except the last one in
                        // the row
        }
      }
      file << "\n";  // End of row
    }
    file.close();
  }
}
void DumpMatchMatrix(colmap::Database& database,
                     const std::vector<Image>& images,
                     const std::filesystem::path& image_path,
                     const std::filesystem::path& output_path) {
  Eigen::MatrixXi match_matrix(images.size(), images.size());
  match_matrix.setZero();
  for (int i = 0; i < images.size(); i++) {
    LOG(INFO) << "Image " << i << " is " << images[i].Name() << " id "
              << images[i].ImageId();
    for (int j = i + 1; j < images.size(); j++) {
      const auto a = images[i].ImageId();
      const auto b = images[j].ImageId();
      if (!database.ExistsMatches(a, b)) {
        continue;
      }
      LOG(INFO) << "  matches Image " << j << " is " << images[j].Name()
                << " id " << images[j].ImageId();
      const std::vector<FeatureMatch> matches = database.ReadMatches(a, b);
      match_matrix(i, j) = matches.size();
      match_matrix(j, i) = matches.size();
    }
  }
  SaveMatrixAsCSV(match_matrix, (output_path / "match_matrix.csv").string());
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  CHECK(!FLAGS_image_path.empty());
  CHECK(!FLAGS_database.empty());

  const std::filesystem::path image_path(FLAGS_image_path);
  CHECK(exists(image_path));

  const std::filesystem::path output_path(FLAGS_output_path);
  if (exists(output_path)) {
    remove_all(output_path);
  }
  std::filesystem::create_directories(output_path);

  colmap::Database database(FLAGS_database);

  std::vector<Camera> cameras = database.ReadAllCameras();
  std::vector<Image> images = database.ReadAllImages();
  const bool dump_kpts = true;
  for (const auto& image : images) {
    LOG(INFO) << "Image ID: " << image.ImageId();
    LOG(INFO) << "Name: " << image.Name();
    LOG(INFO) << "Camera ID: " << image.CameraId();
    const FeatureKeypoints kpts = database.ReadKeypoints(image.ImageId());
    LOG(INFO) << "Keypoints: " << kpts.size();
    const auto image_file = image_path / image.Name();
    if (dump_kpts) {
      cv::Mat im = cv::imread(image_file.string(), cv::IMREAD_UNCHANGED);
      DrawKeypoints(im, kpts);
      cv::imwrite((output_path / image.Name()).string(), im);
    }
  }
  // DumpMatches(database, images, image_path, output_path);
  DumpMatchMatrix(database, images, image_path, output_path);

  database.Close();

  return 0;
}
