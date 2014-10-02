#include "mainwin.h"

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
	ui.cbType->insertSeparator(255);
	ui.cbType->addItem("Solid color",CONV_SOLID_COL);
	ui.cbType->setCurrentIndex(0);
	chaMode();

	isPlaying = false;
	isGif = false;

	connect(ui.tbOpen,SIGNAL(clicked()),this,SLOT(openFile()));
	connect(ui.tbSave,SIGNAL(clicked()),this,SLOT(saveFile()));
	connect(ui.tbSaveAni,SIGNAL(clicked()),this,SLOT(saveAni()));

	connect(ui.tbPrev,SIGNAL(clicked()),this,SLOT(prevFrame()));
	connect(ui.tbNext,SIGNAL(clicked()),this,SLOT(nextFrame()));
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

	connect(ui.brightLevel,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetBrg()));
	connect(ui.contrast,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetCon()));
	connect(ui.sbBlue,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetB()));
	connect(ui.sbRed,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetR()));
	connect(ui.sbGreen,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetG()));
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

void MWin::nextFrame() {
	curFrame++;
	if (curFrame >= gif.size()) curFrame = 0;
	chaFrame();
}

void MWin::prevFrame() {
	curFrame--;
	if (curFrame < 0) curFrame = gif.size() - 1;
	chaFrame();
}

void MWin::chaFrame() {
	if ((curFrame < 0) || (curFrame >= gif.size())) {
		img.fill(Qt::black);
	} else {
		img = gif.at(curFrame).img;
		ui.labFrame->setText(QString("%0 / %1").arg(curFrame + 1).arg(gif.size()));
		chaZoom();
	}
}

void MWin::playGif() {
	isPlaying = !isPlaying;
	ui.tbPlay->setText(isPlaying ? "Stop" : "Play");
	ui.tbNext->setEnabled(!isPlaying);
	ui.tbPrev->setEnabled(!isPlaying);
	QTimer::singleShot(gif.at(curFrame).delay,this,SLOT(playFrame()));
}

void MWin::playFrame() {
	if (!isPlaying) return;
	nextFrame();
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
		default:			// type 0
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
	QString path = QFileDialog::getOpenFileName(this,"Open image","","Images (*.jpg *.jpeg *.png *.bmp *.gif)",0,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	if (path.endsWith(".gif")) {
		gif.clear();
		QImageReader rd(path);
		GIFrame frm;
		while (rd.canRead()) {
			frm.delay = rd.nextImageDelay();
			frm.img = rd.read();
			gif.append(frm);
		}
		isGif = true;
		isPlaying = false;
		curFrame = 0;
		img = gif.at(curFrame).img;
	} else {
		if (img.load(path)) {
			ui.labSrc->dx = 0;
			ui.labSrc->dy = 0;
			isGif = false;
		} else {
			QMessageBox::critical(this,"Error","Fail to load image",QMessageBox::Ok);
		}
	}
	curFrame = 0;
	ui.tbPrev->setEnabled(isGif);
	ui.tbNext->setEnabled(isGif);
	ui.tbPlay->setEnabled(isGif);
	ui.tbSaveAni->setEnabled(isGif);
	ui.labFrame->setText(QString("%0 / %1").arg(curFrame + 1).arg(gif.size()));
	chaZoomMode();
}

void MWin::saveFile() {
	if (img.isNull()) return;
	QString path = QFileDialog::getSaveFileName(this,"Save screen","","ZX screen (*.scr)",0,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QByteArray data = img2scr(dst);
	QFile file(path);
	file.open(QFile::WriteOnly);
	if (file.isOpen()) file.write(data);
}

void MWin::saveAni() {
	if (!isGif) return;
	// QMessageBox::warning(this,"Oops","Not working yet",QMessageBox::Ok);
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

	QString path = QFileDialog::getSaveFileName(this,"Save ani","","ZX animations (*.ani)",0,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(ani);
	} else {
		QMessageBox::critical(this,"Error","Can't open file for writing",QMessageBox::Ok);
	}

	img = gif.at(curFrame).img;
	chaZoom();
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

int getGray(int lev, int brg, int cont) {
	lev = (lev - 128) * cont / 50 + 128;
	lev = lev * brg / 50;
	if (lev < 0) lev = 0;
	if (lev > 255) lev = 255;
	return lev;
}

void poster(QImage& src) {
	int x,y;
	QRgb col;
	int red,grn,blu;
	for (x = 0; x < src.width(); x++) {
		for (y = 0; y < src.height(); y++) {
			col = src.pixel(x,y);
			red = qRed(col) & 0x80;
			grn = qGreen(col) & 0x80;
			blu = qBlue(col) & 0x80;
			col = qRgb(red,grn,blu);
			src.setPixel(x,y,col);
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
		default:
			res = QImage(256,192,QImage::Format_RGB32);
			res.fill(qRgb(0,0,0));
			break;
	}
	return res;
}
