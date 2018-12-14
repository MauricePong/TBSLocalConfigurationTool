#include "hdmioptiongui.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QStyleFactory>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef Q_OS_WIN //windows
	QApplication::setStyle(QStyleFactory::create("windows"));
#else

#endif
	a.setWindowIcon(QIcon::fromTheme(QLatin1String("TBSLocalConfigurationTool"), QIcon(":/image/tbs.ico")));
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    //加载样式表
    QFile file(":/qss/psblack.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
    a.setFont(QFont("Microsoft Yahei", 10));
    AppInit::Instance()->start();

  //  QFont font  = qApp->font();
   //    font.setPointSize(10);
   //    qApp->setFont(font);

    Hdmioptiongui w;
    w.show();

    return a.exec();
}
