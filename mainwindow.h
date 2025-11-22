#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QSpinBox>
#include <QFormLayout>

// 包含你的截图类
#include "screencapturer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onFullScreenCapture();
    void onInteractiveCapture();
    void onRegionCapture();
    void updateStatus(const QString& message);

private:
    void setupUI();

    // UI组件
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;

    QGroupBox* captureGroup;
    QVBoxLayout* captureLayout;

    QPushButton* fullScreenBtn;
    QPushButton* interactiveBtn;
    QPushButton* regionCaptureBtn;

    QGroupBox* regionGroup;
    QFormLayout* regionLayout;
    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    QSpinBox* widthSpinBox;
    QSpinBox* heightSpinBox;

    QLabel* statusLabel;

    // 截图工具
    ScreenCapturer* capturer;
};
#endif // MAINWINDOW_H