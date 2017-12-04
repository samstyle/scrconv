#ifndef MAINWIN_H
#define MAINWIN_H

#include <ui_mainwin.h>
#include <QtGui>
#include <QMimeData>
#include <QClipboard>
#include <QNetworkReply>
#include <QNetworkAccessManager>

enum {
	CONV_REAL=0,
	CONV_WIDTH,
	CONV_HEIGHT,
	CONV_SCALE,
	CONV_ZOOM
};

enum {
	CONV_SEPARATOR = -1,
	CONV_SOLID = 0,
	CONV_TRITONE,
	CONV_TEXTURE,
	CONV_CHUNK4,
	CONV_SOLID_COL,
	CONV_HWMC,
	CONV_3LMC,
	CONV_TRICOLOR,
	CONV_END
};

enum {
	TRI_GRID=0,
	TRI_HLINE,
	TRI_VLINE
};

struct GIFrame {
	int delay;
	QImage img;
};

class MWin : public QMainWindow {
	Q_OBJECT
	public:
		MWin(QWidget* = NULL);
	private:
		Ui::MainWindow ui;
		QImage img;		// original
		QImage src;		// piece for convert
		QImage dst;		// result
		QList<GIFrame> gif;
		int curFrame;
		int convType;
		int scw, sch;
		bool isGif;
		bool isPlaying;
		QImage getSource();

		QByteArray scr;
		QClipboard* cbrd;

		QNetworkAccessManager mng;
		QNetworkReply* rply;

	private slots:
		void openFile(QString p = QString());
		void pasteImage();
		void pasteMime(const QMimeData*);
		void openUrl();
		void downloaded(QNetworkReply*);
		int loadImage(QImageReader&);
		int parseImage(QByteArray&);

		void saveAni();
		void saveScr();
		void savePng();
		void saveBatch();

		void saveScreen(QString,bool);
		void saveHWMC(QString);
		void saveChunk(QString);

		void setFrame(int);
		void playGif();
		void playFrame();

		void setZoom(QAction*);

		void resetB();
		void resetR();
		void resetG();
		void resetBrg();
		void resetCon();
		void resetTMax();
		void resetTMin();

		void convert();
		void chaZoom();
		void chaZoomH();
		void chaZoomW();
		void chaZoomHW();
		void chaZoomOrig();
		void chaZoomFit();
		void chaMode();
	protected:
		void keyPressEvent(QKeyEvent*);
		void dragEnterEvent(QDragEnterEvent*);
		void dropEvent(QDropEvent*);
};

#endif // MAINWIN_H
