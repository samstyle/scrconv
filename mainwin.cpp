#include "mainwin.h"

unsigned char texture_bin[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
  0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x00, 0x00,
  0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
  0x22, 0x00, 0x8a, 0x00, 0x22, 0x00, 0xaa, 0x00,
  0x22, 0x00, 0xaa, 0x00, 0x22, 0x00, 0xaa, 0x00,
  0xaa, 0x00, 0x8a, 0x00, 0xaa, 0x00, 0xaa, 0x00,
  0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00,
  0xaa, 0x00, 0xaa, 0x10, 0xaa, 0x00, 0xaa, 0x00,
  0xaa, 0x44, 0xaa, 0x00, 0xaa, 0x44, 0xaa, 0x00,
  0xaa, 0x44, 0xaa, 0x10, 0xaa, 0x44, 0xaa, 0x00,
  0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11,
  0xaa, 0x55, 0xaa, 0x11, 0xaa, 0x45, 0xaa, 0x11,
  0xaa, 0x55, 0xaa, 0x11, 0xaa, 0x55, 0xaa, 0x11,
  0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x45, 0xaa, 0x55,
  0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55,
  0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x5d, 0xaa, 0x55,
  0xaa, 0x55, 0xaa, 0x77, 0xaa, 0x55, 0xaa, 0x77,
  0xaa, 0xdd, 0xaa, 0x77, 0xaa, 0xdd, 0xaa, 0x77,
  0xaa, 0xdd, 0xaa, 0xf7, 0xaa, 0xdd, 0xaa, 0xff,
  0xaa, 0xdd, 0xaa, 0xff, 0xaa, 0xdd, 0xaa, 0xff,
  0xaa, 0xff, 0xaa, 0xf7, 0xaa, 0xff, 0xaa, 0xff,
  0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff,
  0xaa, 0xff, 0xae, 0xff, 0xaa, 0xff, 0xaa, 0xff,
  0xbb, 0xff, 0xaa, 0xff, 0xbb, 0xff, 0xaa, 0xff,
  0xaa, 0xff, 0xbb, 0xff, 0xae, 0xff, 0xbb, 0xff,
  0xee, 0xff, 0xbb, 0xff, 0xee, 0xff, 0xbb, 0xff,
  0xee, 0xff, 0xfb, 0xff, 0xee, 0xff, 0xff, 0xff,
  0xee, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

MWin::MWin(QWidget* par = NULL):QMainWindow(par) {
	ui.setupUi(this);

	ui.labSrc->setPixmap(QPixmap(256,192));
	ui.labResult->setPixmap(QPixmap(256,192));
/*
	ui.cbScale->addItem("Real size", CONV_REAL);
	ui.cbScale->addItem("Scale to width",CONV_WIDTH);
	ui.cbScale->addItem("Scale to heigth",CONV_HEIGHT);
	ui.cbScale->addItem("Scale to fit",CONV_SCALE);
	ui.cbScale->addItem("Free zoom",CONV_ZOOM);
	ui.cbScale->setCurrentIndex(0);
	zoomMode = CONV_REAL;
*/
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

//	connect(ui.cbScale,SIGNAL(activated(int)),this,SLOT(chaZoomMode()));
	connect(ui.tbSizeH,SIGNAL(released()),this,SLOT(chaZoomH()));
	connect(ui.tbSizeW,SIGNAL(released()),this,SLOT(chaZoomW()));
	connect(ui.tbSizeHW,SIGNAL(released()),this,SLOT(chaZoomHW()));
	connect(ui.tbSizeOrig,SIGNAL(released()),this,SLOT(chaZoomOrig()));
	connect(ui.tbSizeFit,SIGNAL(released()),this,SLOT(chaZoomFit()));
	connect(ui.tbShowGrid,SIGNAL(released()),this,SLOT(convert()));
	connect(ui.cbType,SIGNAL(activated(int)),this,SLOT(chaMode()));

	connect(ui.labSrc,SIGNAL(mMove()),this,SLOT(chaZoom()));
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
	connect(ui.triMax, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetTMax()));
	connect(ui.triMin, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetTMin()));

//	ui.statusbar->showMessage(QString("Qt %0").arg(qVersion()));
}

void MWin::keyPressEvent(QKeyEvent* ev) {
	switch(ev->key()) {
		case Qt::Key_W: ui.labSrc->shift(0, 1); break;
		case Qt::Key_A: ui.labSrc->shift(1, 0); break;
		case Qt::Key_S: ui.labSrc->shift(0, -1); break;
		case Qt::Key_D: ui.labSrc->shift(-1, 0); break;
	}
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
void MWin::resetTMax() {ui.triMax->setValue(224);}
void MWin::resetTMin() {ui.triMin->setValue(128);}

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
	ui.tbPlay->setIcon(QIcon(isPlaying ? ":/stop.png" : ":/play.png"));
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

unsigned char getCols(QImage& src, int x, int y, QRgb& inkcol, QRgb& papcol) {
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
	unsigned char atr;
	atr = (qBlue(inkcol) ? 1 : 0) | (qRed(inkcol) ? 2 : 0) | (qGreen(inkcol) ? 4 : 0);
	atr |= (qBlue(papcol) ? 8 : 0) | (qRed(papcol) ? 16 : 0) | (qGreen(papcol) ? 32 : 0);
	return atr;
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

		ui.spFrame->setEnabled(isGif);
		ui.tbPlay->setEnabled(isGif);
		ui.labFrame->setText(QString(" / %0").arg(gif.size() - 1));

		ui.aSaveAni->setEnabled(isGif);
		ui.aBatchScr->setEnabled(isGif);
		chaZoomHW();
		setWindowTitle(QString("GFXcon [ %0 ]").arg(path));
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
		saveChunk(path);
	} else {
		path = QFileDialog::getSaveFileName(this,"Save screen","","ZX screen (*.scr)");
		if (path.isEmpty()) return;
		saveScreen(path,!ui.aBWscreen->isChecked());
	}
}

void MWin::saveBatch() {
	if (!isGif || (gif.size() == 0)) return;
	QString path;
	if (convType == CONV_CHUNK4) {
		path = QFileDialog::getSaveFileName(this,"Save chunks","","Chunk screen (*.rch)");
	} else {
		path = QFileDialog::getSaveFileName(this,"Save screens","","ZX color screen batch (*.scr)");
	}
	if (path.isEmpty()) return;
	int cnt;
//	QByteArray scr;
	QString fpath;
	bool col = !ui.aBWscreen->isChecked();
	for (cnt = 0; cnt < gif.size(); cnt++) {
		img = gif.at(cnt).img;
		chaZoom();		// dst = converted QImage
		if (convType == CONV_CHUNK4) {
//			scr = img2chunk(dst);
			fpath = path;
			fpath.append(QString(".%0.rch").arg(QString::number(cnt+1000).right(3)));
			saveChunk(fpath);
		} else {
//			scr = img2scr(dst);	// scr = zx screen
			fpath = path;
			fpath.append(QString(".%0.scr").arg(QString::number(cnt+1000).right(3)));
			saveScreen(fpath,col);
		}
	}
	setFrame(curFrame);
}

void MWin::saveChunk(QString path) {
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(rch.data(), 0x300);
	} else {
		QMessageBox::warning(this,"Error","Can't open file for writing",QMessageBox::Ok);
	}
}

void MWin::saveScreen(QString path, bool col) {
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(scr.data(),col ? 0x1b00 : 0x1800);
	} else {
		QMessageBox::warning(this,"Error","Can't open file for writing",QMessageBox::Ok);
	}
}

void MWin::saveAni() {
	if (!isGif) return;
	if (convType == CONV_CHUNK4) return;
	QString path = QFileDialog::getSaveFileName(this,"Save ani","","ZX animations (*.ani)");
	if (path.isEmpty()) return;
	int cnt, type, minSize, bestType;
	int sx = img.width() * ui.labSrc->magX;
	int sy = img.height() * ui.labSrc->magY;
	int posx = (ui.labSrc->dx < 0) ? (-ui.labSrc->dx) & 0xf8 : 0;
	int posy = (ui.labSrc->dy < 0) ? (-ui.labSrc->dy) & 0xf8 : 0;
	int dx = ((sx - ui.labSrc->dx) > 256) ? (256 - posx) : (sx - ((ui.labSrc->dx > 0) ? ui.labSrc->dx : 0));
	int dy = ((sy - ui.labSrc->dy) > 192) ? (192 - posy) : (sy - ((ui.labSrc->dy > 0) ? ui.labSrc->dy : 0));
	posx >>= 3;	// to box
	posy >>= 3;
	if (dx & 7) dx = (dx & 0xf8) + 8;
	if (dy & 7) dy = (dy & 0xf8) + 8;
	dx >>= 3;	// box size
	dy >>= 3;
	const char sign[] = {'G','I','F',' ','a','n','i','m','a','t','i','o','n',0x00};
	QByteArray spr = emptySprite(dx, dy);
	QByteArray pspr;
	QByteArray frm[9];
	QByteArray ani;
	ani.append(sign, 14);
	ani.append((char)(dx & 0xff));
	ani.append((char)((dy << 3) & 0xff));
	bool col = (convType & 256);
	for (cnt = 0; cnt < gif.size(); cnt++) {
		img = gif.at(cnt).img;
		chaZoom();	// scr = converted screen
		pspr = spr;
		spr = cutSprite(scr, posx, posy, dx, dy);
		minSize = 0xffff;
		bestType = 1;
		for (type = 1; type < 9; type++) {
			frm[type] = packSprite(spr,pspr,type,col);
			if (frm[type].size() < minSize) {
				minSize = frm[type].size();
				bestType = type;
			}
		}
		type = gif.at(cnt).delay / 20;
		if (type == 0) type++;
		if (type > 0xff) type = 0xff;
		ani.append(type & 0xff);
		ani.append((bestType & 0x3f) | (col ? 0x80 : 0x00));
		ani.append(frm[bestType]);
	}
	ani.append(0xff);
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

void MWin::chaZoomH() {
	ui.labSrc->magY = 192.0 / img.height();
	ui.labSrc->magX = ui.labSrc->magY;
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	chaZoom();
}

void MWin::chaZoomW() {
	ui.labSrc->magX = 256.0 / img.width();
	ui.labSrc->magY = ui.labSrc->magX;
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	chaZoom();
}

void MWin::chaZoomHW() {
	float mx = 256.0 / img.width();
	float my = 192.0 / img.height();
	ui.labSrc->magX = (mx < my) ? mx : my;
	ui.labSrc->magY = ui.labSrc->magX;
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	chaZoom();
}

void MWin::chaZoomOrig() {
	ui.labSrc->magX = 1.0;
	ui.labSrc->magY = 1.0;
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	chaZoom();
}

void MWin::chaZoomFit() {
	ui.labSrc->magX = 256.0 / img.width();
	ui.labSrc->magY = 192.0 / img.height();
	ui.labSrc->dx = 0;
	ui.labSrc->dy = 0;
	chaZoom();
}

QImage MWin::getSource() {
	int x = ui.labSrc->dx / ui.labSrc->magX;
	int y = ui.labSrc->dy / ui.labSrc->magY;
	int dx = 256 / ui.labSrc->magX;
	int dy = 192 / ui.labSrc->magY;
	return img.copy(x, y, dx, dy).scaled(256, 192);
}

void MWin::chaZoom() {
	src = getSource();
	ui.labSrc->setPixmap(QPixmap::fromImage(src));
	convert();
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

void nextDot(unsigned char& mask, int& adr) {
	mask >>= 1;
	if (mask) return;
	mask = 0x80;
	adr++;
	if (adr & 0x1f) return;
	adr += 0xe0;
	if (adr & 0x700) return;
	adr -= 0x7e0;
	if (adr & 0xe0) return;
	adr += 0x700;
	if (adr < 0x1800) return;
	adr -= 0x1800;
}

QByteArray emptyScreen() {
	QByteArray res(0x1800,0);
	res.append(QByteArray(0x300,7));
	return res;
}

// array to image

QRgb colTab[16] = {
	qRgb(0,0,0), qRgb(0,0,160), qRgb(160,0,0), qRgb(160,0,160),\
	qRgb(0,160,0), qRgb(0,160,160), qRgb(160,160,0), qRgb(160,160,160),\
	qRgb(0,0,0), qRgb(0,0,255), qRgb(255,0,0), qRgb(255,0,255),\
	qRgb(0,255,0), qRgb(0,255,255), qRgb(255,255,0), qRgb(255,255,255)
};

QImage scr2img(QByteArray data) {
	QImage res(256,192,QImage::Format_RGB888);
	res.fill(qRgb(100,100,100));
	int adr = 0;
	int aadr = 0x1800;
	QRgb ink = qRgb(0,0,0);
	QRgb pap = ink;
	int iidx, pidx;
	unsigned char mask = 0x80;
	int x,y;
	for (y = 0; y < 192; y++) {
		for (x = 0; x < 256; x++) {
			if ((x & 7) == 0) {
				iidx = data[aadr] & 7;
				pidx = (data[aadr] & 0x38) >> 3;
				if (data[aadr] & 0x40) {
					iidx |= 8;
					pidx |= 8;
				}
				ink = colTab[iidx];
				pap = colTab[pidx];
				aadr++;
			}
			res.setPixel(x, y, (data[adr] & mask) ? ink : pap);
			nextDot(mask, adr);
		}
		if ((y & 7) != 7) {
			aadr -= 0x20;
		}
	}
	return res;
}

const int dChkTab[] = {0x00, 0x50, 0xa0, 0xf8};

void putChunk(QImage& img, int x, int y, int chk) {
	chk &= 3;
	int sx, sy;
	unsigned char txt;
	for(sy = 0; sy < 4; sy++) {
		for (sx = 0; sx < 4; sx++) {
			txt = texture_bin[dChkTab[chk] | (sy & 3)];
			if (txt & (0x80 >> (sx & 3))) {
				img.setPixel(x + sx, y + sy, qRgb(255,255,255));
			} else {
				img.setPixel(x + sx, y + sy, qRgb(0,0,0));
			}
		}
	}
}

QImage rch2img(QByteArray data) {
	QImage res(256,192,QImage::Format_RGB888);
	int x,y;
	unsigned char dat;
	int adr = 0;
	for (y = 0; y < 192; y += 8) {
		for (x = 0; x < 256; x += 8) {
			dat = data[adr];
			putChunk(res, x, y, dat >> 6);
			putChunk(res, x + 4, y, dat >> 4);
			putChunk(res, x, y + 4, dat >> 2);
			putChunk(res, x + 4, y + 4, dat);
			adr++;
		}
	}
	return res;
}

// solid

QByteArray doSolid(QImage& src) {
	QByteArray res = emptyScreen();
	int x,y;
	int xmax = src.width();
	int ymax = src.height();
	int adr = 0;
	unsigned char mask = 0x80;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			if (qGray(src.pixel(x,y)) >= 128) {
				res[adr] = res[adr] | mask;
			}
			nextDot(mask, adr);
		}
	}
	return res;
}

// solid.color

QByteArray doSolidCol(QImage src) {
	QByteArray res = emptyScreen();
	poster(src);
	int x,y,lin,bit;
	unsigned char atr, scrbyte;
	QRgb col, ink, pap;
	int xmax = src.width();
	int ymax = src.height();
	int adr = 0;
	int aadr = 0x1800;
	for (y = 0; y < ymax; y += 8) {
		for (x = 0; x < xmax; x += 8) {
			atr = getCols(src, x, y, ink, pap);
			res[aadr] = atr;
			for (lin = 0; lin < 8; lin++) {
				scrbyte = 0;
				for (bit = 0; bit < 8; bit++) {
					scrbyte <<= 1;
					col = src.pixel(x + bit, y + lin);
					scrbyte |= (col == ink) ? 1 : 0;
				}
				res[adr + (lin << 8)] = scrbyte;
			}
			adr++;
			if ((adr & 0xff) == 0x00) {
				adr += 0x700;
				if (adr > 0x1800) {
					adr -= 0x1800;
				}
			}
			aadr++;
		}
	}
	return res;
}

// tritone

QByteArray doTritone(QImage& src, int lev1, int lev2) {
	QByteArray res = emptyScreen();
	int x,y,lev;
	if (lev1 > lev2) {
		lev = lev1;
		lev1 = lev2;
		lev2 = lev;
	}
	int xmax = src.width();
	int ymax = src.height();
//	if (xmax > 256) xmax = 256;
//	if (ymax > 192) ymax = 192;
	unsigned char mask = 0x80;
	int adr = 0;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(src.pixel(x,y));
			if (lev < lev1) {
				/* no dot */
			} else if (lev > lev2) {
				res[adr] = res[adr] | mask;
			} else if ((x ^ y) & 1) {
				res[adr] = res[adr] | mask;
			}
			nextDot(mask, adr);
		}
	}
	return res;
}

// texture

QByteArray doTexture(QImage& src) {
	QByteArray res = emptyScreen();
	int x,y,lev;
	int xmax = src.width();
	int ymax = src.height();
//	if (xmax > 256) xmax = 256;
//	if (ymax > 192) ymax = 192;
	unsigned char mask = 0x80;
	int adr = 0;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(src.pixel(x,y));
			lev = texture_bin[(lev & 0xf8) | (y & 7)];
			if (lev & (0x80 >> (x & 7))) {
				res[adr] = res[adr] | mask;
			}
			nextDot(mask, adr);
		}
	}
	return res;
}

QByteArray doChunk44(QImage& src) {
	QByteArray res(0x300,0);
	QImage chsc = src.scaled(64,48);
	int x,y;
	int xmax = chsc.width();
	int ymax = chsc.height();
	int lev;
	int chk;
	int pos;
	unsigned char dat;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(chsc.pixel(x,y));
			chk = (lev >> 6) & 3;
			pos = ((x & 0x3e) >> 1) | ((y & 0x3e) << 4);
			dat = res[pos];
			if (y & 1) {
				if (x & 1) dat = (dat & 0xfc) | chk;
				else dat = (dat & 0xf3) | (chk << 2);
			} else {
				if (x & 1) dat = (dat & 0xcf) | (chk << 4);
				else dat = (dat & 0x3f) | (chk << 6);
			}
			res[pos] = dat;
		}
	}
	return res;
}

QByteArray getConverted(QImage toc, int convType, int l1, int l2) {
	QByteArray res;
	switch (convType) {
		case CONV_SOLID:
			res = doSolid(toc);
			break;
		case CONV_TRITONE:
			res = doTritone(toc, l1, l2);
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
			res = emptyScreen();
			break;
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
			toc.setPixel(x,y,col);
		}
	}
	QByteArray dat = getConverted(toc, convType, ui.triMin->value(), ui.triMax->value());
	switch (convType) {
		case CONV_CHUNK4:
			rch = dat;
			res = rch2img(dat);
			break;
		default:
			scr = dat;
			res = scr2img(dat);
			break;
	}
	if (ui.tbShowGrid->isChecked()) {
		QPainter pnt;
		pnt.begin(&res);
		pnt.setPen(qRgb(60,60,60));
		for (x = 0; x < res.width(); x += 8) {
			pnt.drawLine(x, 0, x, res.height());
		}
		for (y = 0; y < res.height(); y += 8) {
			pnt.drawLine(0, y, res.width(), y);
		}
		pnt.end();
	}

	return res;
}
