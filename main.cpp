#include <mainwin.h>

void MLabel::mousePressEvent(QMouseEvent *ev) {
	switch (ev->button()) {
		case Qt::LeftButton:
			posx = ev->x();
			posy = ev->y();
			break;
		case Qt::RightButton:
			dx = (dx | 7) - 7;
			dy = (dy | 7) - 7;
			emit mMove();
			break;
		case Qt::MiddleButton:
			dx = (dx + 128) / magn - 128;
			dy = (dy + 96) / magn - 96;
			magn = 1.0;
			emit mZoom();
			break;
		default:
			break;
	}
}

void MLabel::mouseMoveEvent(QMouseEvent *ev) {
	if (!(ev->buttons() & Qt::LeftButton)) return;
	dx -= (ev->x() - posx);
	dy -= (ev->y() - posy);
	posx = ev->x();
	posy = ev->y();
	emit mMove();
}

void MLabel::wheelEvent(QWheelEvent* ev) {
	if (blockWheel) return;
	float oldMag = magn;
	if (ev->delta() < 0) {
		if (magn > 0.1) magn /= 1.1;
	} else {
		if (magn < 4.0) magn *= 1.1;
	}
	dx = (dx + 128) * magn / oldMag - 128;
	dy = (dy + 96) * magn / oldMag - 96;
	emit mZoom();
}

int main(int ac, char** av) {
	QApplication app(ac,av);
	MWin win(NULL);
	win.show();
	return app.exec();
}
