#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QTimer>

#include "ScreenCapturer.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("Screenshot Tool (Qt Simple Version)");
        resize(300, 200);

        auto* layout = new QVBoxLayout(this);

        auto* btnFull = new QPushButton("Full Screen Screenshot");
        auto* btnRegion = new QPushButton("Interactive Region Screenshot");
        auto* btnExit = new QPushButton("Exit");

        layout->addWidget(btnFull);
        layout->addWidget(btnRegion);
        layout->addWidget(btnExit);

        connect(btnFull, &QPushButton::clicked, this, [this]() {
            // 立即隐藏窗口
            this->hide();

            // 处理所有挂起的事件，确保隐藏完成
            QApplication::processEvents();
            QApplication::sendPostedEvents();

            // 稍微延迟确保窗口完全隐藏
            QTimer::singleShot(50, this, [this]() {
                ScreenCapturer cap;
                bool success = cap.CaptureFullScreen(L"fullscreen.png");

                // 恢复窗口
                this->show();
                this->raise();
                this->activateWindow();

                if (success) {
                    QMessageBox::information(this, "Success", "Full screen screenshot saved as fullscreen.png");
                    QFileInfo file("fullscreen.png");
                    if (file.exists()) {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(file.absoluteFilePath()));
                    }
                }
                else {
                    QMessageBox::warning(this, "Failed", "Full screen screenshot failed!");
                }
                });
            });

        connect(btnRegion, &QPushButton::clicked, this, [this]() {
            // 立即隐藏窗口
            this->hide();

            // 处理所有挂起的事件，确保隐藏完成
            QApplication::processEvents();
            QApplication::sendPostedEvents();

            // 稍微延迟确保窗口完全隐藏
            QTimer::singleShot(300, this, [this]() {
                bool ok = InteractiveRegionCapturer::StartInteractiveCapture(L"region.png");

                // 恢复窗口
                this->show();
                this->raise();
                this->activateWindow();

                if (ok) {
                    QMessageBox::information(this, "Success", "Region screenshot saved as region.png");
                    QFileInfo file("region.png");
                    if (file.exists()) {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(file.absoluteFilePath()));
                    }
                }
                else {
                    QMessageBox::information(this, "Cancelled", "Region screenshot cancelled!");
                }
                });
            });

        connect(btnExit, &QPushButton::clicked, this, &QApplication::quit);
    }
};
int main(int argc, char* argv[])
{
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    int ret = a.exec();

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return ret;
}

#include "main.moc"