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
#define CONV_CHUNK4 3
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
		QImage src;		// piece for convert
		QImage dst;		// result
		QList<GIFrame> gif;
		int curFrame;
		int convType;
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
