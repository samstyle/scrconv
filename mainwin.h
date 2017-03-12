#ifndef MAINWIN_H
#define MAINWIN_H

#include <ui_mainwin.h>
#include <QtGui>

enum {
	CONV_REAL=0,
	CONV_WIDTH,
	CONV_HEIGHT,
	CONV_SCALE,
	CONV_ZOOM
};

enum {
	CONV_SOLID=0,
	CONV_TRITONE,
	CONV_TEXTURE,
	CONV_CHUNK4,
	CONV_SOLID_COL=256
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
		QImage doConvert(QImage);

		QByteArray scr;
		QByteArray rch;

	private slots:
		void openFile();
		void saveAni();
		void saveScr();
		void savePng();
		void saveBatch();

		void saveScreen(QString,bool);
		void saveChunk(QString);

		void setFrame(int);
		void playGif();
		void playFrame();

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
};

#endif // MAINWIN_H
