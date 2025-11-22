#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), capturer(new ScreenCapturer())
{
    setupUI();
    setWindowTitle("截图工具");
    setFixedSize(400, 400);
}

MainWindow::~MainWindow()
{
    delete capturer;
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // 截图方式选择组
    captureGroup = new QGroupBox("截图方式", this);
    captureLayout = new QVBoxLayout(captureGroup);

    fullScreenBtn = new QPushButton("全屏截图", this);
    interactiveBtn = new QPushButton("交互式区域截图", this);
    regionCaptureBtn = new QPushButton("指定区域截图", this);

    captureLayout->addWidget(fullScreenBtn);
    captureLayout->addWidget(interactiveBtn);
    captureLayout->addWidget(regionCaptureBtn);

    // 区域截图参数组
    regionGroup = new QGroupBox("区域截图参数", this);
    regionLayout = new QFormLayout(regionGroup);

    xSpinBox = new QSpinBox(this);
    ySpinBox = new QSpinBox(this);
    widthSpinBox = new QSpinBox(this);
    heightSpinBox = new QSpinBox(this);

    // 设置SpinBox的范围
    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    xSpinBox->setRange(0, screenGeometry.width());
    ySpinBox->setRange(0, screenGeometry.height());
    widthSpinBox->setRange(1, screenGeometry.width());
    heightSpinBox->setRange(1, screenGeometry.height());

    // 设置默认值
    xSpinBox->setValue(0);
    ySpinBox->setValue(0);
    widthSpinBox->setValue(800);
    heightSpinBox->setValue(600);

    regionLayout->addRow("X坐标:", xSpinBox);
    regionLayout->addRow("Y坐标:", ySpinBox);
    regionLayout->addRow("宽度:", widthSpinBox);
    regionLayout->addRow("高度:", heightSpinBox);

    // 状态标签
    statusLabel = new QLabel("就绪", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; padding: 5px; }");

    // 添加到主布局
    mainLayout->addWidget(captureGroup);
    mainLayout->addWidget(regionGroup);
    mainLayout->addWidget(statusLabel);

    // 连接信号槽
    connect(fullScreenBtn, &QPushButton::clicked, this, &MainWindow::onFullScreenCapture);
    connect(interactiveBtn, &QPushButton::clicked, this, &MainWindow::onInteractiveCapture);
    connect(regionCaptureBtn, &QPushButton::clicked, this, &MainWindow::onRegionCapture);
}

void MainWindow::onFullScreenCapture()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存全屏截图", "", "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (filename.isEmpty()) {
        return;
    }

    updateStatus("正在截取全屏...");

    if (capturer->CaptureFullScreen(filename)) {
        updateStatus("全屏截图保存成功: " + filename);
        QMessageBox::information(this, "成功", "全屏截图已保存: " + filename);
    }
    else {
        updateStatus("全屏截图失败");
        QMessageBox::warning(this, "错误", "全屏截图失败");
    }
}

void MainWindow::onInteractiveCapture()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存区域截图", "", "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (filename.isEmpty()) {
        return;
    }

    updateStatus("请用鼠标拖动选择截图区域...");

    // 隐藏主窗口以便截图
    this->hide();
    QApplication::processEvents(); // 处理事件，确保窗口隐藏

    bool result = InteractiveRegionCapturer::StartInteractiveCapture(filename);

    this->show(); // 重新显示窗口

    if (result) {
        updateStatus("区域截图保存成功: " + filename);
        QMessageBox::information(this, "成功", "区域截图已保存: " + filename);
    }
    else {
        updateStatus("区域截图已取消");
    }
}

void MainWindow::onRegionCapture()
{
    int x = xSpinBox->value();
    int y = ySpinBox->value();
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();

    QString filename = QFileDialog::getSaveFileName(this, "保存区域截图", "", "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (filename.isEmpty()) {
        return;
    }

    updateStatus(QString("正在截取区域 (%1,%2 %3x%4)...").arg(x).arg(y).arg(width).arg(height));

    if (capturer->CaptureSpecificArea(x, y, width, height, filename)) {
        updateStatus("区域截图保存成功: " + filename);
        QMessageBox::information(this, "成功", "区域截图已保存: " + filename);
    }
    else {
        updateStatus("区域截图失败");
        QMessageBox::warning(this, "错误", "区域截图失败");
    }
}

void MainWindow::updateStatus(const QString& message)
{
    statusLabel->setText(message);
    statusLabel->repaint();
}