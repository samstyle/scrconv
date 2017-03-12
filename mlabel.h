#ifndef MLABEL_H
#define MLABEL_H

#include <QLabel>

class MLabel : public QLabel {
	Q_OBJECT
	public:
		MLabel(QWidget* par):QLabel(par) {dx = 0; dy = 0; magX = magY = 1.0;}
		int dx,dy;
		float magX;
		float magY;
		void shift(int, int);
		void setMag(double, double);
	signals:
		void mMove();
		void mZoom();
	private:
		int posx,posy;
	protected:
		void wheelEvent(QWheelEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mousePressEvent(QMouseEvent*);
};

#endif // MLABEL_H
