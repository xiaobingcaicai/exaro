/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include <QtCore>
#include <QBrush>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "detail.h"

inline void initMyResource()
{
	Q_INIT_RESOURCE(detail);
}

Detail::Detail(QGraphicsItem* parent, QObject* parentObject): BandInterface(parent, parentObject)
{
	initMyResource();
	setBandType(Report::BandInterface::Detail);
	setResizeFlags(FixedPos | ResizeBottom);
}

Detail::~Detail()
{
}

bool Detail::canContain(QObject * object)
{
	return (!dynamic_cast<Report::BandInterface*>(object) && dynamic_cast<Report::ItemInterface*>(object));
}

QRectF Detail::boundingRect() const
{
	return QRectF(0, 0, width(), height());
}

void Detail::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{

	QRectF rect = (option->type == QStyleOption::SO_GraphicsItem) ? boundingRect() : option->exposedRect;

	setupPainter(painter);
	painter->fillRect(rect,painter->brush());

	if (option->type == QStyleOption::SO_GraphicsItem)
	{
		drawSelection(painter, rect);
		painter->drawText(rect, Qt::AlignRight | Qt::AlignTop, tr("Detail"));
	}

	adjustRect(rect);

	if (frame()&DrawLeft)
		painter->drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());

	if (frame()&DrawRight)
		painter->drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

	if (frame()&DrawTop)
		painter->drawLine(rect.left(), rect.top(), rect.right(), rect.top());

	if (frame()&DrawBottom)
		painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());


	if (option->type != QStyleOption::SO_GraphicsItem)
		emit afterPrint(this);
}

QIcon Detail::toolBoxIcon()
{
	return QIcon(":/detail.png");
}

QString Detail::toolBoxText()
{
	return tr("Detail");
}

QString Detail::toolBoxGroup()
{
	return tr("Bands");
}

QObject * Detail::createInstance(QGraphicsItem* parent, QObject* parentObject)
{
	return new Detail(parent, parentObject);
}

Q_EXPORT_PLUGIN2(detail, Detail)

