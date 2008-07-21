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

#ifndef TEXT_H
#define TEXT_H

#include <iteminterface.h>


class Text : public Report::ItemInterface
{
	Q_OBJECT
	Q_INTERFACES(Report::ItemInterface);

	Q_FLAGS(TextFlags);
	Q_PROPERTY(TextFlags textFlags READ textFlags WRITE setTextFlags)
	Q_PROPERTY(QString text READ text WRITE setText)

public:

	enum TextFlag {AlignLeft = Qt::AlignLeft,
	               AlignRight = Qt::AlignRight,
	               AlignHCenter = Qt::AlignHCenter,
	               AlignJustify = Qt::AlignJustify,
	               AlignTop = Qt::AlignTop,
	               AlignBottom = Qt::AlignBottom,
	               AlignVCenter = Qt::AlignVCenter,
	               AlignCenter = Qt::AlignCenter,
	               TextDontClip = Qt::TextDontClip,
	               TextSingleLine = Qt::TextSingleLine,
	               TextExpandTabs = Qt::TextExpandTabs,
	               TextShowMnemonic = Qt::TextShowMnemonic,
	               TextWordWrap = Qt::TextWordWrap
	              };
	Q_DECLARE_FLAGS(TextFlags, TextFlag);

public:
	Text(QGraphicsItem* parent = 0, QObject* parentObject = 0);

	QRectF boundingRect() const;
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

	QIcon toolBoxIcon();
	QString toolBoxText();
	QString toolBoxGroup();

	TextFlags textFlags();
	void setTextFlags(TextFlags textFlags);

	QString text();
	void setText(const QString &text);

	QObject * createInstance(QGraphicsItem* parent = 0, QObject* parentObject = 0);

private:
	TextFlags m_textFlags;
	QString m_text;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Text::TextFlags);

#endif
