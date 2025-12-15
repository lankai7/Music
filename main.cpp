/**
 * @brief   : 程序入口
 * @author  : 樊晓亮
 * @date    : 2025.12.12
 **/
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // 启用 Qt 高 DPI 缩放
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);      // 整体缩放
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);        // 图片不模糊

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/image/favicon.ico"));
    MainWindow w;
    w.show();

    return a.exec();
}
