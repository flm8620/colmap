#pragma once
#include "imagescene.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT

  ImageScene scene;

  void ApplyScene();

 public:
  MainWindow(QWidget* parent = nullptr);
 public slots:
  void on_actionOpen_COLMAP_project_triggered();
  void WhenImageSelected(int row, int col);
  void WhenMatchedImageSelected(int row, int col);
  void on_size_slider_valueChanged(int size);
  void on_button_hide_feature_marker_toggled(bool checked);
  void on_button_show_only_matched_toggled(bool checked);
  void on_button_show_match_line_toggled(bool checked);
  void on_button_show_epipolar_toggled(bool checked);
};
