#include "MainWindow.h"
#include "AppTheme.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char* argv[])
{
    // High-DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps,  true);

    // Default OpenGL format with MSAA
    QSurfaceFormat fmt;
    fmt.setSamples(4);
    fmt.setDepthBufferSize(24);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("MuonSimViewer");
    app.setApplicationVersion("4.0");
    app.setOrganizationName("CCTVal / KIT");

    Theme::apply(app);

    MainWindow window;
    window.show();

    // If a file was passed on command line, load it
    if (argc > 1) {
        window.loadFile(QString::fromLocal8Bit(argv[1]));
    }

    return app.exec();
}
