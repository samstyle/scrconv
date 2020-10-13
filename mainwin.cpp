#include "mainwin.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

struct convMethod {
	int id;
	QString name;
	QByteArray(*conv)(QImage&);
	QImage(*getimg)(QByteArray);
	void(*save)(QByteArray);
};

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


// zoom

void MWin::setZoom(QAction* act) {
	int zf = act->data().toInt();
	if ((zf < 1) || (zf > 3)) return;
	scw = 256 * zf;
	sch = 192 * zf;
	ui.labResult->setFixedSize(scw, sch);
	ui.labResult->setPixmap(QPixmap::fromImage(dst).scaled(scw, sch));
	adjustSize();
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
	int brg = (qBlue(col) > 160) ? 1 : 0;
	brg += (qRed(col) > 160) ? 1 : 0;
	brg += (qGreen(col) > 160) ? 1 : 0;
	if (brg > 1) res |= 8;
	return res;
}

unsigned char getCols(QImage& src, int x, int y, QRgb& inkcol, QRgb& papcol, int lmask = 0xff, int wid = 8) {
	QMap<QRgb,int> map;
	QRgb col;
	int lin,bit;
	if (wid < 8) wid = 8;
	if (wid > 256) wid = 256;
	// make box color map
	for (lin = 0; lin < wid; lin++) {
		if (lmask & 0x80) {
			for (bit = 0; bit < 8; bit++) {
				col = src.pixel(x + bit, y + lin);
				map[col]++;
			}
		}
		lmask <<= 1;
	}
	// select 2 colors
	QList<QRgb> cols = map.keys();
	int ink = 0;
	int pap = 0;
	inkcol = qRgb(0,0,0);
	papcol = inkcol;
	foreach(col, cols) {
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
		col = inkcol;
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

int MWin::parseImage(QByteArray& data) {
	QBuffer buf(&data);
	buf.open(QIODevice::ReadOnly);
	QImageReader rd(&buf);
	int res = loadImage(rd);
	buf.close();
	return res;
}

int MWin::loadImage(QImageReader& rd) {
	int res;
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
		res = 1;
	} else {
		QMessageBox::critical(this,"Error","Fail to load image",QMessageBox::Ok);
		res = 0;
	}
	return res;
}

void MWin::openFile(QString path) {
	if (isPlaying) playGif();
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(this,"Open image","","Images (*.jpg *.jpeg *.png *.bmp *.gif)");
	if (path.isEmpty()) return;
	QImageReader rd(path);
	if (loadImage(rd))
		setWindowTitle(QString("GFXcon [ %0 ]").arg(path));
}

void MWin::pasteImage() {
	const QMimeData* mime = cbrd->mimeData();
	pasteMime(mime);
}

void MWin::pasteMime(const QMimeData* mime) {
	if (mime->hasImage()) {
		gif.clear();
		img = qvariant_cast<QImage>(mime->imageData());
		ui.labSrc->dx = 0;
		ui.labSrc->dy = 0;
		isGif = false;
		ui.spFrame->setEnabled(false);
		ui.tbPlay->setEnabled(false);
		ui.labFrame->setText(QString(" / %0").arg(gif.size() - 1));
		ui.aSaveAni->setEnabled(false);
		ui.aBatchScr->setEnabled(false);
		chaZoomHW();
		setWindowTitle(QString("GFXcon [ nonamed ]"));
	} else {
		QMessageBox::critical(this,"Error","Fail to paste image",QMessageBox::Ok);
	}
}

void MWin::openUrl() {
	QString path = QInputDialog::getText(this, "Input URL", "URL");
	if (path.isEmpty()) return;
	QUrl url = QUrl::fromEncoded(path.toLocal8Bit());
	QNetworkRequest rq(url);
	rply = mng.get(rq);
}

void MWin::downloaded(QNetworkReply* data) {
	QByteArray arr = data->readAll();
	if (parseImage(arr))
		setWindowTitle(QString("GFXcon [ %0 ]").arg(data->url().toString()));
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
	QString path = QFileDialog::getSaveFileName(this,"Save...");
	if (path.isEmpty()) return;
	QFile file(path);
	switch (convType) {
		case CONV_CHUNK4:
			saveChunk(path);
			break;
		case CONV_HWMC:
			saveHWMC(path);
			break;
		case CONV_3LMC:
			saveScreen(path, 0);
			break;
		case CONV_TRICOLOR:
			if (file.open(QFile::WriteOnly)) {
				file.write(scr.data(), scr.size());
			} else {
				QMessageBox::warning(this,"Error","Can't open file for writing",QMessageBox::Ok);
			}
			break;
		default:
			saveScreen(path,!ui.aBWscreen->isChecked());
			break;
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
		file.write(scr.data(), 0x300);
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

void MWin::saveHWMC(QString path) {
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		file.write(scr.data(), 0x3000);
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
	QImage res;
	if (img.isNull()) {
		res = QImage(256,192,QImage::Format_RGB888);
		res.fill(Qt::black);
	} else {
		int x = ui.labSrc->dx / ui.labSrc->magX;
		int y = ui.labSrc->dy / ui.labSrc->magY;
		int dx = 256 / ui.labSrc->magX;
		int dy = 192 / ui.labSrc->magY;
		res = img.copy(x, y, dx, dy).scaled(256, 192);
	}
	return res;
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
	int min,max;
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
		res = qRgb(red & 0x80, grn & 0x80, blu & 0x80);
	}
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

QImage hwmc2img(QByteArray data) {
	QImage res(256,192,QImage::Format_RGB888);
	res.fill(qRgb(100,100,100));
	int adr = 0;
	int x,y;
	unsigned char iidx,pidx;
	unsigned char mask = 0x80;
	QRgb ink = qRgb(0,0,0);
	QRgb pap = ink;
	for (y = 0; y < 192; y++) {
		for (x = 0; x < 256; x++) {
			if ((x & 7) == 0) {
				iidx = data[adr + 0x1800] & 7;
				pidx = (data[adr  + 0x1800] & 0x38) >> 3;
				if (data[adr + 0x1800] & 0x40) {
					iidx |= 8;
					pidx |= 8;
				}
				ink = colTab[iidx];
				pap = colTab[pidx];
			}
			res.setPixel(x, y, (data[adr] & mask) ? ink : pap);
			nextDot(mask, adr);
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

QByteArray doSolidCol(QImage& src) {
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

int lev1, lev2, trit, tinv;

QByteArray doTritone(QImage& src) {
	QByteArray res = emptyScreen();
	int x,y,lev;
	if (lev1 > lev2) {
		lev = lev1;
		lev1 = lev2;
		lev2 = lev;
	}
	int xmax = src.width();
	int ymax = src.height();
	unsigned char mask = 0x80;
	int adr = 0;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(src.pixel(x,y));
			if (lev < lev1) {
				/* no dot */
			} else if (lev > lev2) {
				res[adr] = res[adr] | mask;
			} else {
				switch (trit) {
					case TRI_GRID:
						if (((x ^ y) & 1) ^ tinv) res[adr] = res[adr] | mask;
						break;
					case TRI_HLINE:
						if ((y & 1) ^ tinv) res[adr] = res[adr] | mask;
						break;
					case TRI_VLINE:
						if ((x & 1) ^ tinv) res[adr] = res[adr] | mask;
						break;
				}
			}
			nextDot(mask, adr);
		}
	}
	return res;
}

// texture

int txdepth = 4;

int txd_mask[] = {0x80,0xc0,0xe0,0xf0,0xf8};

QByteArray doTexture(QImage& src) {
	QByteArray res = emptyScreen();
	int x,y,lev;
	int xmax = src.width();
	int ymax = src.height();
	unsigned char mask = 0x80;
	int adr = 0;
	if (txdepth > 4) txdepth = 4;
	if (txdepth < 0) txdepth = 0;
	int txmask = txd_mask[txdepth];
	int txbits = txmask ^ 0xf8;
	int idx;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			lev = qGray(src.pixel(x,y));
			idx = (lev & txmask) | (y & 7);
			if (lev & 0x80) idx |= txbits;
			lev = texture_bin[idx];
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

// atr/8dot multicolor

QByteArray doHWMC(QImage& src) {
	QByteArray res(0x3000, 0x00);
	QRgb col;
	poster(src);
	int xmax = src.width();
	int ymax = src.height();
	int x,y, bit;
	int adr = 0;
	unsigned char mask = 0x01;
	unsigned char atr;
	unsigned char scrbyte;
	QRgb ink, pap;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x += 8) {
			atr = getCols(src, x, y, ink, pap, 0x80);		// 8 dot atr
			scrbyte = 0;
			for (bit = 0; bit < 8; bit++) {
				scrbyte <<= 1;
				col = src.pixel(x + bit, y);
				scrbyte |= (col == ink) ? 1 : 0;
			}
			res[adr] = scrbyte;
			res[adr + 0x1800] = atr;
			mask = 0x00;
			nextDot(mask, adr);
		}
	}
	return res;
}

// 3Lines RGB multicolor

QByteArray do3LMC(QImage& src) {
	QByteArray res(0x1800, 0x00);
	QRgb col;
	poster(src);
	int xmax = src.width();
	int ymax = src.height();
	int x,y;
	unsigned char bt;
	unsigned char mask = 0x80;
	int adr = 0;
	int lev;
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			col = src.pixel(x, y);
			switch(y % 3) {
				case 0: lev = qRed(col); break;
				case 1: lev = qGreen(col); break;
				default: lev = qBlue(col); break;
			}
			if (lev & 0x80) {
				bt = res[adr];
				bt |= mask;
				res[adr] = bt;

			}
			nextDot(mask, adr);
		}
	}
	return res;
}

QImage tlmc2img(QByteArray data) {
	QImage res(256, 192, QImage::Format_RGB888);
	res.fill(Qt::black);
	QRgb col;
	int adr = 0;
	unsigned char mask = 0x80;
	int x,y;
	for (y = 0; y < 192; y++) {
		switch (y % 3) {
			case 0: col = qRgb(160, 0, 0); break;
			case 1: col = qRgb(0, 160, 0); break;
			default: col = qRgb(0, 0, 160); break;
		}
		for (x = 0; x < 256; x++) {
			if (data[adr] & mask) {
				res.setPixel(x, y, col);
			}
			nextDot(mask, adr);
		}
	}
	return res;
}

// tricolor

QByteArray do3col(QImage& src) {
	QByteArray res(0x4800, 0x00);
	// poster(src);
	int adr = 0;
	unsigned char mask = 0x80;
	QRgb col;
	int x,y;
	int xmax = src.width();
	int ymax = src.height();
	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++) {
			col = src.pixel(x, y);
			if (qRed(col) & 0x80)
				res[adr] = res[adr] | mask;
			if (qGreen(col) & 0x80)
				res[adr + 0x1800] = res[adr + 0x1800] | mask;
			if (qBlue(col) & 0x80)
				res[adr + 0x3000] = res[adr + 0x3000] | mask;
			nextDot(mask, adr);
		}
	}
	return res;
}

QImage rgb2img(QByteArray data) {
	QImage res(256, 192, QImage::Format_RGB888);
	res.fill(Qt::black);
	if (data.size() < 0x4800) return res;
	int adr = 0;
	unsigned char mask = 0x80;
	int x,y;
	int r,g,b;
	for (y = 0; y < 192; y++) {
		for (x = 0; x < 256; x++) {
			r = (data[adr] & mask) ? 0xa0 : 0x00;
			g = (data[adr + 0x1800] & mask) ? 0xa0 : 0x00;
			b = (data[adr + 0x3000] & mask) ? 0xa0 : 0x00;
			res.setPixel(x, y, qRgb(r,g,b));
			nextDot(mask, adr);
		}
	}
	return res;
}

//

static convMethod convTab[] = {
	{CONV_SOLID, "Solid", doSolid, scr2img, NULL},
	{CONV_TRITONE, "Tritone", doTritone, scr2img, NULL},
	{CONV_TEXTURE, "Texture", doTexture, scr2img, NULL},
	{CONV_CHUNK4, "Chunk 4x4", doChunk44, rch2img, NULL},
	{CONV_SEPARATOR, "", NULL, NULL, NULL},
	{CONV_SOLID_COL, "Solid color", doSolidCol, scr2img, NULL},
	{CONV_SEPARATOR, "", NULL, NULL, NULL},
	{CONV_HWMC, "HW multicolor", doHWMC, hwmc2img, NULL},
	{CONV_3LMC, "3Line multicolor", do3LMC, tlmc2img, NULL},
	{CONV_SEPARATOR, "", NULL, NULL, NULL},
	{CONV_TRICOLOR, "3-color", do3col, rgb2img, NULL},
	{CONV_END, "", NULL, NULL, NULL}
};

convMethod* findMethod(int type) {
	int idx = 0;
	while ((convTab[idx].id != CONV_END) && (convTab[idx].id != type)) {
		idx++;
	}
	return &convTab[idx];
}

void MWin::convert() {
	if (src.isNull()) {
		dst = QImage(256, 192, QImage::Format_RGB888);
		dst.fill(Qt::black);
	} else {
		int x,y,red,grn,blu;
		QRgb col;
		QImage toc = src;
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
		lev1 = ui.triMin->value();
		lev2 = ui.triMax->value();
		trit = ui.cbTriType->itemData(ui.cbTriType->currentIndex()).toInt();
		tinv = ui.cbInvertGrid->isChecked() ? 1 : 0;
		txdepth = ui.texdepth->value();
		convMethod* mtd = findMethod(convType);
		if (mtd == NULL) {
			dst = QImage(256, 192, QImage::Format_RGB888);
			dst.fill(Qt::gray);
		} else {
			if (mtd->conv) {
				scr = mtd->conv(toc);
				if (mtd->getimg) {
					dst = mtd->getimg(scr);
				} else {
					dst = QImage(256, 192, QImage::Format_RGB888);
					dst.fill(Qt::gray);
				}
			} else {
				scr.clear();
				dst = QImage(256, 192, QImage::Format_RGB888);
				dst.fill(Qt::gray);
			}
		}
		if (ui.tbShowGrid->isChecked()) {
			QPainter pnt;
			pnt.begin(&dst);
			pnt.setPen(qRgb(60,60,60));
			for (x = 0; x < dst.width(); x += 8) {
				pnt.drawLine(x, 0, x, dst.height());
			}
			for (y = 0; y < dst.height(); y += 8) {
				pnt.drawLine(0, y, dst.width(), y);
			}
			pnt.end();
		}
	}
	ui.labResult->setFixedSize(scw,sch);
	ui.labResult->setPixmap(QPixmap::fromImage(dst).scaled(scw, sch));
	resize(minimumSize());
}

MWin::MWin(QWidget* par):QMainWindow(par) {
	ui.setupUi(this);

	cbrd = QApplication::clipboard();

	scw = 256 * 2;
	sch = 192 * 2;

	ui.labSrc->setPixmap(QPixmap(256,192));
	ui.labResult->setPixmap(QPixmap(256,192));

	int idx = 0;
	while (convTab[idx].id != CONV_END) {
		if (convTab[idx].id == CONV_SEPARATOR) {
			ui.cbType->insertSeparator(255);
		} else {
			ui.cbType->addItem(convTab[idx].name, convTab[idx].id);
		}
		idx++;
	}
	ui.cbType->setCurrentIndex(0);
	chaMode();

	ui.actX1->setData(1);
	ui.actX2->setData(2);
	ui.actX3->setData(3);
	ui.tbZoom->addAction(ui.actX1);
	ui.tbZoom->addAction(ui.actX2);
	ui.tbZoom->addAction(ui.actX3);

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

	ui.cbTriType->addItem("Grid",TRI_GRID);
	ui.cbTriType->addItem("HLines",TRI_HLINE);
	ui.cbTriType->addItem("VLines",TRI_VLINE);

	ui.aSaveAni->setEnabled(false);
	ui.aBatchScr->setEnabled(false);

	isPlaying = false;
	isGif = false;
	curFrame = 0;

	connect(ui.tbOpen,SIGNAL(clicked()),this,SLOT(openFile()));
	connect(ui.tbPaste,SIGNAL(clicked()),this,SLOT(pasteImage()));
	connect(ui.tbUrl,SIGNAL(clicked()),this,SLOT(openUrl()));

	connect(ui.aSaveScr,SIGNAL(triggered()),this,SLOT(saveScr()));
	connect(ui.aSaveAni,SIGNAL(triggered()),this,SLOT(saveAni()));
	connect(ui.aBatchScr,SIGNAL(triggered()),this,SLOT(saveBatch()));
	connect(ui.aSavePng,SIGNAL(triggered()),this,SLOT(savePng()));

	connect(ui.spFrame,SIGNAL(valueChanged(int)),this,SLOT(setFrame(int)));
	connect(ui.tbPlay,SIGNAL(clicked()),this,SLOT(playGif()));

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
	connect(ui.cbTriType,SIGNAL(currentIndexChanged(int)),this,SLOT(convert()));
	connect(ui.cbInvertGrid,SIGNAL(stateChanged(int)), this, SLOT(convert()));
	connect(ui.texdepth,SIGNAL(valueChanged(int)), this, SLOT(convert()));

	connect(ui.brightLevel,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetBrg()));
	connect(ui.contrast,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetCon()));
	connect(ui.sbBlue,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetB()));
	connect(ui.sbRed,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetR()));
	connect(ui.sbGreen,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetG()));
	connect(ui.triMax, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetTMax()));
	connect(ui.triMin, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(resetTMin()));

	connect(ui.tbZoom, SIGNAL(triggered(QAction*)), this, SLOT(setZoom(QAction*)));

	connect(&mng,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloaded(QNetworkReply*)));

//	ui.statusbar->showMessage(QString("Qt %0").arg(qVersion()));
}

void MWin::keyPressEvent(QKeyEvent* ev) {
	if (ev->modifiers() & Qt::ControlModifier) {
		switch (ev->key()) {
			case Qt::Key_1: ui.labSrc->setMag(1.0, 1.0); break;
			case Qt::Key_2: ui.labSrc->setMag(0.5, 0.5); break;
			case Qt::Key_3: ui.labSrc->setMag(0.3, 0.3); break;
			case Qt::Key_4: ui.labSrc->setMag(0.25, 0.25); break;
			case Qt::Key_O: openFile(); break;
			case Qt::Key_V: pasteImage(); break;
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch(ev->key()) {
			case Qt::Key_1: setZoom(ui.actX1); break;
			case Qt::Key_2: setZoom(ui.actX2); break;
			case Qt::Key_3: setZoom(ui.actX3); break;
		}
	} else {
		switch(ev->key()) {
			case Qt::Key_W: ui.labSrc->shift(0, 1); break;
			case Qt::Key_A: ui.labSrc->shift(1, 0); break;
			case Qt::Key_S: ui.labSrc->shift(0, -1); break;
			case Qt::Key_D: ui.labSrc->shift(-1, 0); break;
			case Qt::Key_1: ui.labSrc->setMag(1.0, 1.0); break;
			case Qt::Key_2: ui.labSrc->setMag(2.0, 2.0); break;
			case Qt::Key_3: ui.labSrc->setMag(3.0, 3.0); break;
			case Qt::Key_4: ui.labSrc->setMag(4.0, 4.0); break;
		}
	}
}

// drag'n'drop

void MWin::dragEnterEvent(QDragEnterEvent* ev) {
	ev->acceptProposedAction();
}

void MWin::dropEvent(QDropEvent* ev) {
	const QMimeData* mime = ev->mimeData();
	if (mime->hasUrls())
		openFile(mime->urls().first().path());
}
