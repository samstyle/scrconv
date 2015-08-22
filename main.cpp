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
			dx = (dx + 128) / magX - 128;
			dy = (dy + 96) / magY - 96;
			magX = magY = 1.0;
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
//	if (blockWheel) return;
	float oldMagX = magX;
	float oldMagY = magY;
	if (ev->delta() < 0) {
		if ((magX > 0.1) && (magY > 0.1)) {
			magX /= 1.1;
			magY /= 1.1;
		}
	} else {
		if ((magX < 4.0) && (magY < 4.0)) {
			magX *= 1.1;
			magY *= 1.1;
		}
	}
	dx = (dx + 128) * magX / oldMagX - 128;
	dy = (dy + 96) * magY / oldMagY - 96;
	emit mZoom();
}

int main(int ac, char** av) {
	QApplication app(ac,av);
	MWin win(NULL);
	win.show();
	return app.exec();
}
