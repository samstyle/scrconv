#include "mainwin.h"

QByteArray chunkScr(768, 0);

MWin::MWin(QWidget* par = NULL):QMainWindow(par) {
	ui.setupUi(this);

	ui.labSrc->setPixmap(QPixmap(256,192));
	ui.labResult->setPixmap(QPixmap(256,192));

	ui.cbScale->addItem("Real size", CONV_REAL);
	ui.cbScale->addItem("Scale to width",CONV_WIDTH);
	ui.cbScale->addItem("Scale to heigth",CONV_HEIGHT);
	ui.cbScale->addItem("Scale to fit",CONV_SCALE);
	ui.cbScale->addItem("Free zoom",CONV_ZOOM);
	ui.cbScale->setCurrentIndex(0);
	zoomMode = CONV_REAL;

	ui.cbType->addItem("Solid",CONV_SOLID);
	ui.cbType->addItem("Tritone",CONV_TRITONE);
	ui.cbType->addItem("Texture",CONV_TEXTURE);
	ui.cbType->addItem("Chunks 4x4",CONV_CHUNK4);
	ui.cbType->insertSeparator(255);
	ui.cbType->addItem("Solid color",CONV_SOLID_COL);
	ui.cbType->setCurrentIndex(0);
	chaMode();

	QAction* sep1 = new QAction(NULL);
	QAction* sep2 = new QAction(NULL);
	sep1->setSeparator(true);
	sep2->setSeparator(true);
	ui.tbSave->addAction(ui.aBWscreen);
	ui.tbSave->addAction(sep1);
	ui.tbSave->addAction(ui.aSaveScr);
	ui.tbSave->addAction(ui.aSavePng);
	ui.tbSave->addAction(sep2);
	ui.tbSave->addAction(ui.aSaveAni);
	ui.tbSave->addAction(ui.aBatchScr);

	ui.aSaveAni->setEnabled(false);
	ui.aBatchScr->setEnabled(false);

	isPlaying = false;
	isGif = false;
	curFrame = 0;

	connect(ui.tbOpen,SIGNAL(clicked()),this,SLOT(openFile()));

	connect(ui.aSaveScr,SIGNAL(triggered()),this,SLOT(saveScr()));
	connect(ui.aSaveAni,SIGNAL(triggered()),this,SLOT(saveAni()));
	connect(ui.aBatchScr,SIGNAL(triggered()),this,SLOT(saveBatch()));
	connect(ui.aSavePng,SIGNAL(triggered()),this,SLOT(savePng()));

	connect(ui.spFrame,SIGNAL(valueChanged(int)),this,SLOT(setFrame(int)));
	connect(ui.tbPlay,SIGNAL(clicked()),this,SLOT(playGif()));

	connect(ui.cbScale,SIGNAL(activated(int)),this,SLOT(chaZoomMode()));
	connect(ui.cbType,SIGNAL(activated(int)),this,SLOT(chaMode()));

	connect(ui.labSrc,SIGNAL(mMove()),this,SLOT(movePic()));
	connect(ui.labSrc,SIGNAL(mZoom()),this,SLOT(chaZoom()));

	connect(ui.brightLevel,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.contrast,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.triMax,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.triMin,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.sbRed,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.sbGreen,SIGNAL(valueChanged(int)),this,SLOT(convert()));
	connect(ui.sbBlue,SIGNAL(valueChanged(int)),this,SLOT(convert()));
//	connect(ui.tbCrop,SIGNAL(toggled(bool)),this,SLOT(setCrop()));

	connect(ui.brightLevel,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetBrg()));
	connect(ui.contrast,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetCon()));
	connect(ui.sbBlue,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetB()));
	connect(ui.sbRed,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetR()));
	connect(ui.sbGreen,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetG()));

	ui.statusbar->showMessage(QString("Qt %0").arg(qVersion()));
}

// reset levels

void centerSlider(QSlider* sld) {
	sld->setValue(sld->maximum() / 2);
}

void MWin::resetBrg() {centerSlider(ui.brightLevel);}
void MWin::resetCon() {centerSlider(ui.contrast);}
void MWin::resetB() {centerSlider(ui.sbBlue);}
void MWin::resetR() {centerSlider(ui.sbRed);}
void MWin::resetG() {centerSlider(ui.sbGreen);}

// gif

void MWin::setFrame(int nr) {
	if (!isGif) return;			// fuck off
	if (gif.size() == 0) return;
	if ((nr < 0) || (nr >= gif.size())) {
		while (nr < 0) nr += gif.size();
		while (nr >= gif.size()) nr -= gif.size();
		curFrame = nr;
		ui.spFrame->setValue(nr);
	} else {
		curFrame = nr;
		img = gif.at(curFrame).img;
		ui.labFrame->setText(QString(" / %0").arg(gif.size() - 1));
		chaZoom();
	}
}

void MWin::playGif() {
	isPlaying = !isPlaying;
	ui.tbPlay->setText(isPlaying ? "Stop" : "Play");
//	ui.tbNext->setEnabled(!isPlaying);
//	ui.tbPrev->setEnabled(!isPlaying);
	QTimer::singleShot(gif.at(curFrame).delay,this,SLOT(playFrame()));
}

void MWin::playFrame() {
	if (!isPlaying) return;
	ui.spFrame->setValue(curFrame+1);		// it connected to setFrame(int);
	QTimer::singleShot(gif.at(curFrame).delay,this,SLOT(playFrame()));
}

// common

int getCol(QRgb col) {
	int res = 0;
	if (qBlue(col) > 80) res |= 1;
	if (qRed(col) > 80) res |= 2;
	if (qGreen(col) > 80) res |= 4;
//	if ((qBlue(col) > 160) || (qRed(col) > 160) || (qGreen(col > 160))) res |= 8;
	return res;
}

void getCols(QImage& src, int x, int y, QRgb& inkcol, QRgb& papcol) {
	QMap<QRgb,int> map;
	int lin,bit;
	// make box color map
	for (lin = 0; lin < 8; lin++) {
		for (bit = 0; bit < 8; bit++) {
			map[src.pixel(x + bit, y + lin)]++;
		}
	}
	// select 2 colors
	QList<QRgb> cols = map.keys();
	int ink = 0;
	int pap = 0;
	inkcol = qRgb(0,0,0);
	papcol = inkcol;
	foreach(QRgb col, cols) {
		if (map.value(col) > ink) {
			pap = ink;
			ink = map.value(col);
			papcol = inkcol;
			inkcol = col;
		} else if (map.value(col) > pap) {
			papcol = col;
			pap = map.value(col);
		}
	}
	if (inkcol == qRgb(0,0,0)) {
		QRgb col = inkcol;
		inkcol = papcol;
		papcol = col;
	}
	if (papcol == inkcol) {
		papcol = qRgb(0,0,0);
	}
}

QByteArray img2scr(QImage& src) {
	QByteArray res(0x1800,0x00);
	QByteArray atrs(0x300,0x47);
	QRgb inkcol,papcol;
	int adr,atradr;
	int x,y,lin,bit,ink,pap,atr;
	for (y = 0; y < 192; y += 8) {
		for (x = 0; x < 256; x +=8) {
			getCols(src,x,y,inkcol,papcol);
			ink = getCol(inkcol);
			pap = getCol(papcol);
			if (((ink & 7) == 0) && ((pap & 7) == 0)) {
				atr = 0x07;
				ink = 7;
				inkcol = qRgb(255,255,255);
			} else {
				atr = (ink & 7) | ((pap & 7) << 3);
				if ((ink | pap) & 0x08) atr |= 0x40;
			}
			atradr = ((y & 0xf8) << 2) | ((x & 0xf8) >> 3);
			atrs[atradr] = atr;

			adr = ((y & 0xc0) << 5) | ((y & 0x38) << 2) | ((y & 7) << 8) | ((x & 0xf8) >> 3);
			for (lin = 0; lin < 8; lin++) {
				for (bit = 0; bit < 8; bit++) {
					if (src.pixel(x + bit,y + lin) == inkcol) {
						res[adr + (lin << 8)] = res[adr + (lin << 8)] | (0x80 >> bit);
					}
				}
			}
		}
	}
	res.append(atrs);
	return res;
}

// pack animation

QByteArray getXorBox(QByteArray spr, int adr) {
	QByteArray res;
	char bt = 0;
	for (int tmp = 0; tmp < 8; tmp++) {
		res.append(spr[adr] ^ bt);
		bt = spr[adr];
		adr++;
	}
	res.append(spr[adr]);	// color
	return res;
}

int bitCount(char bt) {
	int res = 0;
	int mask = 0x80;
	while (mask) {
		if (bt & mask) res++;
		mask >>= 1;
	}
	return res;
}

void xorSprite(QByteArray& spr, QByteArray& prv) {
	int adr = 0;
	int tmp;
	while (adr < spr.size()) {
		for (tmp = 0; tmp < 8; tmp++) {		// xor 8 bytes with prev.spr
			spr[adr] = spr[adr] ^ prv[adr];
			adr++;
		}
		adr++;		// skip color
	}
}

QByteArray packPass2(QByteArray& data, bool col) {
	QByteArray res;
	int adr = 0;
	int len;
	int mask = 0x100;
	int tmp = 0;
	while (adr < data.size()) {
		if (mask > 0x80) {
			tmp = res.size();
			res.append((char)0x00);		// flag2
			mask = 0x01;
		}
		if (data[adr]) {
			res[tmp] = res[tmp] | (mask & 0xff);
			len = bitCount(data[adr]);
			res.append(data.mid(adr,len+1));
			adr += len;
		}
		adr++;
		if (col) {
			res.append(data[adr]);
			adr++;
		}
		mask <<= 1;
	}
	return res;
}

QByteArray packSprite(QByteArray spr, QByteArray& prv, int type, bool col) {
	int adr;
	int tmp;
	int cnt;
	// unsigned char flag;
	char bt;
	QByteArray res,qba;
	QByteArray xorbox;
	switch (type) {
		case 3:
			xorSprite(spr,prv);
		case 1:
			adr = 0;
			do {
				xorbox = getXorBox(spr,adr);
				bt = 0;
				tmp = res.size();
				res.append((char)0x00);
				for (cnt = 0; cnt < 8; cnt++) {
					if (xorbox[cnt] != bt) {
						res[tmp] = res[tmp] | (0x01 << cnt);
						res.append(xorbox[cnt]);
					}
					bt = xorbox[cnt];
				}
				if (col) res.append(xorbox[8]);
				adr += 9;
			} while (adr < spr.size());
			break;
		case 4:
			xorSprite(spr,prv);
		case 2:
			adr = 0;
			do {
				xorbox = getXorBox(spr,adr);
				tmp = res.size();
				res.append((char)0x00);
				for (cnt = 0; cnt < 8; cnt++) {
					if (xorbox.at(cnt) != 0x00) {
						res[tmp] = res[tmp] | (0x01 << cnt);
						res.append(xorbox[cnt]);
					}
				}
				if (col) res.append(xorbox[8]);
				adr += 9;
			} while (adr < spr.size());
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			qba = packSprite(spr,prv,type - 4,col);
			res = packPass2(qba,col);
			break;
		default:			// type 0 TODO:convert boxed sprite to line format (need dx at least)
/*
			adr = 0;
			do {
				res.append(scr.mid(adr,32));
				adr += 0x100;
				if ((adr & 0x700) == 0) {
					adr -= 0x7E0;		// next box line
					if ((adr & 0xff) == 0) {
						adr += 0x700;	// next 1/3 of screen
					}
				}
			} while (adr < 0x1800);
*/
			break;
	}
	return res;
}

QByteArray cutSprite(QByteArray& scr, int posx, int posy, int dx, int dy) {
	QByteArray res;
	int adr = ((posy & 0x18) << 8) | ((posy & 7) << 5) | (posx & 0x1f);
	int atr = ((posy & 0x1f) << 5) | (posx & 0x1f) | 0x1800;
	int cx,cy,cl;
	int tadr,tatr;
	for (cy = 0; cy < dy; cy++) {
		tadr = adr;
		tatr = atr;
		for (cx = 0; cx < dx; cx++) {
			for (cl = 0; cl < 0x800; cl += 0x100) {
				res.append(scr[tadr + cl]);
			}
			res.append(scr[tatr]);
			tadr++;
			tatr++;
		}
		atr += 0x20;
		adr += 0x20;
		if (adr & 0x700) adr = (adr & 0x181f) + 0x800;
	}
	return res;
}

QByteArray emptySprite(int dx, int dy) {
	int size = dx * dy * 9;
	QByteArray res(size,0x00);
	return res;
}

// load-save

void MWin::openFile() {
	if (isPlaying) playGif();
	QString path = QFileDialog::getOpenFileName(this,"Open image","","Images (*.jpg *.jpeg *.png *.bmp *.gif)");
	if (path.isEmpty()) return;
	QImageReader rd(path);
	if (rd.canRead()) {
		gif.clear();
		curFrame = 0;
		if (rd.format() == "gif") {
			GIFrame frm;
			while (rd.canRead()) {
				frm.delay = rd.nextImageDelay();
				frm.img = rd.read();
				gif.append(frm);
			}
			isGif = true;
			img = gif.at(curFrame).img;
			ui.spFrame->setMinimum(-1);			// +- 1 from real min/max values for cycling
			ui.spFrame->setMaximum(gif.size());
			ui.spFrame->setValue(curFrame);
		} else {
			img = rd.read();
			ui.labSrc->dx = 0;
			ui.labSrc->dy = 0;
			isGif = false;
		}
		isPlaying = false;
//		ui.tbCrop->setChecked(false);

		ui.spFrame->setEnabled(isGif);
		ui.tbPlay->setEnabled(isGif);
		ui.labFrame->setText(QString(" / %0").arg(gif.size() - 1));

		ui.aSaveAni->setEnabled(isGif);
		ui.aBatchScr->setEnabled(isGif);
		chaZoomMode();
	} else {
		QMessageBox::critical(this,"Error","Fail to load image",QMessageBox::Ok);
	}
}

void MWin::savePng() {
	if (img.isNull()) return;
	QString path = QFileDialog::getSaveFileName(this,"Save screen","","PNG fils (*.png)");
	if (path.isEmpty()) return;
	QImageWriter wr(path);
	wr.setFormat("png");
	wr.write(dst);
}

void MWin::saveScr() {
	if (img.isNull()) return;
	QString path;
	if (convType == CONV_CHUNK4) {
		path = QFileDialog::getSaveFileName(this,"Save chunks","","Chunk screen (*.rch)");
		if (path.isEmpty()) return;
		QFile file(path);
		if (file.open(QFile::WriteOnly)) {
			file.write(chunkScr.data(), 0x300);
		} else {
			QMessageBox::warning(this,"Error","Can't open file for writing",QMessageBox::Ok);
		}
	} else {
		path = QFileDialog::getSaveFileName(this,"Save screen","","ZX screen (*.scr)");
		if (path.isEmpty()) return;
		saveScreen(path,!ui.aBWscreen->isChecked());
	}
}

void MWin::saveBatch() {
	if (!isGif || (gif.size() == 0)) return;
	QString path = QFileDialog::getSaveFileName(this,"Save screens","","ZX color screen batch (*.scr)");
	if (path.isEmpty()) return;
	int cnt;
	QByteArray scr;
	QString fpath;
	bool col = !ui.aBWscreen->isChecked();
	for (cnt = 0; cnt < gif.size(); cnt++) {
		img = gif.at(cnt).img;
		chaZoom();		// dst = converted QImage
		scr = img2scr(dst);	// scr = zx screen
		fpath = path;
		fpath.append(QString(".%0.scr").arg(QString::number(cnt+1000).right(3)));
		saveScreen(fpath,col);
	}
	setFrame(curFrame);
}

void MWin::saveScreen(QString path, bool col) {
	QByteArray data = img2scr(dst);
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(data.data(),col ? 0x1b00 : 0x1800);
	} else {
		QMessageBox::warning(this,"Error","Can't open file for writing",QMessageBox::Ok);
	}
}

void MWin::saveAni() {
	if (!isGif) return;
	QString path = QFileDialog::getSaveFileName(this,"Save ani","","ZX animations (*.ani)");
	if (path.isEmpty()) return;
	int cnt, type, minSize, bestType;
	int posx = (ui.labSrc->dx < 0) ? (-ui.labSrc->dx) & 0xf8 : 0;
	int posy = (ui.labSrc->dy < 0) ? (-ui.labSrc->dy) & 0xf8 : 0;
	int dx = ((imgScaled.width() - ui.labSrc->dx) > 256) ? (256 - posx) : (imgScaled.width() - ((ui.labSrc->dx > 0) ? ui.labSrc->dx : 0));
	int dy = ((imgScaled.height() - ui.labSrc->dy) > 192) ? (192 - posy) : (imgScaled.height() - ((ui.labSrc->dy > 0) ? ui.labSrc->dy : 0));
	posx >>= 3;	// to box
	posy >>= 3;
	if (dx & 7) dx = (dx & 0xf8) + 8;
	if (dy & 7) dy = (dy & 0xf8) + 8;
	dx >>= 3;	// box size
	dy >>= 3;
	const char sign[] = {'G','I','F',' ','a','n','i','m','a','t','i','o','n',0x00};
	QByteArray scr(0x1b00,0x00);
	QByteArray pspr;
	QByteArray spr = emptySprite(dx,dy);
	QByteArray frm[9];
	QByteArray ani;
	ani.append(sign,14);
	ani.append((char)(dx & 0xff));
	ani.append((char)((dy << 3) & 0xff));
	bool col = (convType & 256);
	for (cnt = 0; cnt < gif.size(); cnt++) {
		img = gif.at(cnt).img;
		chaZoom();		// dst = converted QImage
		pspr = spr;
		scr = img2scr(dst);	// zx screen
		spr = cutSprite(scr,posx,posy,dx,dy);	// cut sprite for convert
		minSize = 0xffff;
		bestType = 1;
		for (type = 1; type < 9; type++) {
			frm[type] = packSprite(spr,pspr,type,col);
			if (frm[type].size() < minSize) {
				minSize = frm[type].size();
				bestType = type;
			}
		}
//		bestType = 1;
//		qDebug() << "frm" << cnt << "best pack" << bestType << "size" << minSize;
		type = gif.at(cnt).delay / 20;
		if (type == 0) type++;
		if (type > 0xff) type = 0xff;
		ani.append(type & 0xff);
		ani.append((bestType & 0x3f) | (col ? 0x80 : 0x00));
		ani.append(frm[bestType]);
	}
	ani.append(0xff);

//	QString path = QFileDialog::getSaveFileName(this,"Save ani","","ZX animations (*.ani)");
//	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(ani);
	} else {
		QMessageBox::critical(this,"Error","Can't open file for writing",QMessageBox::Ok);
	}
	setFrame(curFrame);
}

// change parameters

void MWin::chaMode() {
	convType = ui.cbType->itemData(ui.cbType->currentIndex()).toInt();
	ui.triMax->setEnabled(convType == CONV_TRITONE);
	ui.triMin->setEnabled(convType == CONV_TRITONE);
	ui.sbRed->setEnabled(convType & 256);
	ui.sbGreen->setEnabled(convType & 256);
	ui.sbBlue->setEnabled(convType & 256);
	convert();
}

void MWin::chaZoomMode() {
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	zoomMode = ui.cbScale->itemData(ui.cbScale->currentIndex()).toInt();
	chaZoom();
}

void MWin::chaZoom() {
	int x,y;
	switch (zoomMode) {
		case CONV_REAL:
			imgScaled = img;
			break;
		case CONV_WIDTH:
			imgScaled = img.scaledToWidth(256);
			break;
		case CONV_HEIGHT:
			imgScaled = img.scaledToHeight(192);
			break;
		case CONV_SCALE:
			imgScaled = img.scaled(256,192,Qt::KeepAspectRatio);
			break;
		case CONV_ZOOM:
			x = img.width() * ui.labSrc->magn;
			y = img.height() * ui.labSrc->magn;
			imgScaled = img.scaled(x,y,Qt::KeepAspectRatio);
			break;
	}
	movePic();
}

void MWin::movePic() {
	if (img.isNull()) return;
	int dx = ui.labSrc->dx;
	int dy = ui.labSrc->dy;
	switch(zoomMode) {
		case CONV_WIDTH:
			dx = 0;
			break;
		case CONV_HEIGHT:
			dy = 0;
			break;
		case CONV_SCALE:
			dx = 0;
			dy = 0;
			break;
	}
	src = imgScaled.copy(dx, dy, 256, 192);
	ui.labSrc->setPixmap(QPixmap::fromImage(src));
	ui.labSrc->blockWheel = (zoomMode != CONV_ZOOM);
	convert();
}

void MWin::setCrop() {
}

int getGray(int lev, int brg, int cont) {
	lev = (lev - 128) * cont / 50 + 128;
	lev = lev * brg / 50;
	if (lev < 0) lev = 0;
	if (lev > 255) lev = 255;
	return lev;
}

QRgb rgb2zx(QRgb col) {
	QRgb res;
/*
	int y = 0.299 * red + 0.587 * grn + 0.114 * blu;
	int u = -0.14713 * red - 0.28886 * grn + 0.436 * blu + 128;
	int v = 0.615 * red - 0.51499 * grn - 0.10001 * blu + 128;
*/
	int min,max;	// h
	int red = qRed(col);
	int grn = qGreen(col);
	int blu = qBlue(col);

	if (red < grn) {
		min = (red < blu) ? red : blu;
		max = (grn < blu) ? blu : grn;
	} else {
		min = (grn < blu) ? grn : blu;
		max = (red < blu) ? blu : red;
	}
	if ((max - min) < 32) {
		res = (qGray(col) > 128) ? qRgb(128,128,128) : qRgb(0,0,0);
	} else {
		res = qRgb(red & 128, grn & 128, blu & 128);
	}

/*
	if (min == max) {
		h = 0;
	} else {
		if (max == red) {
			h = 60 * (grn - blu) / (max - min);
			if (h < 0) h += 360;
		} else if (max == grn) {
			h = 60 * (blu - red) / (max - min) + 120;
		} else {
			h = 60 * (red - grn) / (max - min) + 240;
		}
	}

	if (max < 80) {
		res = qRgb(0,0,0);
	} else if ((max - min) < 64) {
		res = (max > 128) ? qRgb(128,128,128) : qRgb(0,0,0);
	} else {
		if (h < 30) res = qRgb(128,0,0);	// red (orange)
		else if (h < 90) res = qRgb(128,128,0);	// yellow
		else if (h < 150) res = qRgb(0,128,0);	// green
		else if (h < 210) res = qRgb(0,128,128);// cyan
		else if (h < 270) res = qRgb(0,0,128);	// blue
		else if (h < 330) res = qRgb(128,0,128);// magenta
		else res = qRgb(128,0,0);		// red again
	}

//	res = qRgb(red & 128, grn & 128, blu & 128);
*/
	return res;
}

void poster(QImage& src) {
	int x,y;
//	QRgb col;
	for (x = 0; x < src.width(); x++) {
		for (y = 0; y < src.height(); y++) {
			src.setPixel(x,y,rgb2zx(src.pixel(x,y)));
		}
	}
}

// solid

QImage doSolid(QImage& src) {
	QImage res(256,192,QImage::Format_RGB32);
	int x,y;
	int xmax = src.width();
	int ymax = src.height();
	if (xmax > 256) xmax = 256;
	if (ymax > 192) ymax = 192;
	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			if (qGray(src.pixel(x,y)) >= 128) {
				res.setPixel(x,y,qRgb(255,255,255));
			} else {
				res.setPixel(x,y,qRgb(0,0,0));
			}
		}
	}
	return res;
}

// solid.color

QImage doSolidCol(QImage src) {
	QImage res(256,192,QImage::Format_RGB32);
	poster(src);
	int x,y,lin,bit;
	QRgb inkcol,papcol,col;
	int xmax = src.width();
	int ymax = src.height();
	if (xmax > 256) xmax = 256;
	if (ymax > 192) ymax = 192;
	for (x = 0; x < xmax; x += 8) {
		for (y = 0; y < ymax; y += 8) {
			getCols(src,x,y,inkcol,papcol);
			for (lin = 0; lin < 8; lin++) {
				for (bit = 0; bit < 8; bit++) {
					col = src.pixel(x + bit, y + lin);
					if (col == inkcol) {
						res.setPixel(x + bit, y + lin, inkcol);
					} else {
						res.setPixel(x + bit, y + lin, papcol);
					}
				}
			}
		}
	}
	return res;
}

// tritone

QImage doTritone(QImage& src, int lev1, int lev2) {
	QImage res(256,192,QImage::Format_RGB32);
	int x,y,lev;
	if (lev1 > lev2) {
		lev = lev1;
		lev1 = lev2;
		lev2 = lev;
	}
	int xmax = src.width();
	int ymax = src.height();
	if (xmax > 256) xmax = 256;
	if (ymax > 192) ymax = 192;
	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			lev = qGray(src.pixel(x,y));
			if (lev < lev1) {
				res.setPixel(x,y,qRgb(0,0,0));
			} else if (lev > lev2) {
				res.setPixel(x,y,qRgb(255,255,255));
			} else {
				if ((x ^ y) & 1) res.setPixel(x,y,qRgb(255,255,255));
				else res.setPixel(x,y,qRgb(0,0,0));
			}
		}
	}
	return res;
}

// texture

#include "textures.h"

QImage doTexture(QImage& src) {
	QImage res(256,192,QImage::Format_RGB32);
	int x,y,lev;
	int xmax = src.width();
	int ymax = src.height();
	if (xmax > 256) xmax = 256;
	if (ymax > 192) ymax = 192;
	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			lev = qGray(src.pixel(x,y));
			lev = texture_bin[(lev & 0xf8) | (y & 7)];
			if (lev & (0x80 >> (x & 7))) {
				res.setPixel(x,y,qRgb(255,255,255));
			} else {
				res.setPixel(x,y,qRgb(0,0,0));
			}
		}
	}
	return res;
}

/*
int getGraySum(QImage& src, int x, int y, int dx, int dy) {
	int res = 0;
	int sx, sy;
	for (sx = 0; sx < dx; sx++) {
		for (sy = 0; sy < dy; sy++) {
			res += qGray(src.pixel(x + sx, y + sy));
		}
	}
	return res;
}
*/

const int dChkTab[] = {0x00, 0x50, 0xa0, 0xf8};

QImage doChunk44(QImage& src) {
	QImage res(256,192,QImage::Format_RGB32);
	QImage chsc = src.scaled(64,48);
	int x,y;
	int lev = 0;
	int chk = 0;
	int txt;
	int pos,dat;
	int xmax = src.width();
	int ymax = src.height();
	if (xmax > 256) xmax = 256;
	if (ymax > 192) ymax = 192;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(chsc.pixel(x >> 2, y >> 2));
			chk = (lev >> 6) & 3;
			txt = texture_bin[dChkTab[chk] | (y & 3)];
			if (txt & (0x80 >> (x & 3))) {
				res.setPixel(x,y,qRgb(255,255,255));
			} else {
				res.setPixel(x,y,qRgb(0,0,0));
			}
			if (((x & 3) == 0) && ((y & 3) == 0)) {
				pos = ((x & 0xf8) >> 3) | ((y & 0xf8) << 2);
				dat = chunkScr[pos];
				if (y & 4) {
					if (x & 4) dat = (dat & 0xfc) | chk;
					else dat = (dat & 0xf3) | (chk << 2);
				} else {
					if (x & 4) dat = (dat & 0xcf) | (chk << 4);
					else dat = (dat & 0x3f) | (chk << 6);
				}
				chunkScr[pos] = dat;
			}
		}
	}
	return res;
}

void MWin::convert() {
	if (src.isNull()) return;
	dst = doConvert(src);
	ui.labResult->setPixmap(QPixmap::fromImage(dst));
}

QImage MWin::doConvert(QImage toc) {
	int x,y,red,grn,blu;
	QImage res;
	QRgb col;
	int rlev = ui.sbRed->value();
	int glev = ui.sbGreen->value();
	int blev = ui.sbBlue->value();
	int brg = ui.brightLevel->value();
	int con = ui.contrast->value();
	for (y = 0; y < toc.height(); y++) {
		for (x = 0; x < toc.width(); x++) {
			col = toc.pixel(x,y);
			red = getGray(qRed(col) * rlev / 100,brg,con);
			grn = getGray(qGreen(col) * glev / 100,brg,con);
			blu = getGray(qBlue(col) * blev / 100,brg,con);
			col = qRgb(red,grn,blu);
			toc.setPixel(x,y,qRgb(red,grn,blu));
		}
	}
	switch (convType) {
		case CONV_SOLID:
			res = doSolid(toc);
			break;
		case CONV_TRITONE:
			res = doTritone(toc, ui.triMin->value(), ui.triMax->value());
			break;
		case CONV_TEXTURE:
			res = doTexture(toc);
			break;
		case CONV_SOLID_COL:
			res = doSolidCol(toc);
			break;
		case CONV_CHUNK4:
			res = doChunk44(toc);
			break;
		default:
			res = QImage(256,192,QImage::Format_RGB32);
			res.fill(qRgb(0,0,0));
			break;
	}
	return res;
}
