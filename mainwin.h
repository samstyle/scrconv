#ifndef MAINWIN_H
#define MAINWIN_H

#include <ui_mainwin.h>
#include <QtGui>

#define CONV_REAL 0
#define CONV_WIDTH 1
#define CONV_HEIGHT 2
#define CONV_SCALE 3
#define	CONV_ZOOM 4

#define	CONV_SOLID 0
#define CONV_TRITONE 1
#define CONV_TEXTURE 2
#define CONV_SOLID_COL 256

struct GIFrame {
	int delay;
	QImage img;
};

class MWin : public QMainWindow {
	Q_OBJECT
	public:
		MWin(QWidget*);
	private:
		Ui::MainWindow ui;
		QImage img;		// original
		QImage imgScaled;	// scaled
		QImage src;		// piece for convert
		QImage dst;		// result
		QList<GIFrame> gif;
		int curFrame;
		int zoomMode;
		int convType;
		bool isGif;
		bool isPlaying;
		void chaFrame();
		QImage doConvert(QImage);
	private slots:
		void openFile();
		void saveFile();
		void saveAni();

		void prevFrame();
		void nextFrame();
		void playGif();
		void playFrame();

		void resetB();
		void resetR();
		void resetG();
		void resetBrg();
		void resetCon();

		void convert();
		void movePic();
		void chaZoom();
		void chaMode();
		void chaZoomMode();
};

#endif // MAINWIN_H
