/***************************************************************************
 *   Copyright (C) 2009 by Alexander Mikhalov                              *
 *   alexmi3@rambler.ru                                                    *
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

#include "command.h"
#include <QGraphicsScene>
#include <QGraphicsView>


AddCommand::AddCommand( Report::PageInterface* page, const char* itemClassName, QPointF pos, mainWindow* mw )
{
	m_mainWindow = mw;
	m_pos = pos;
	m_itemClassName = itemClassName;
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );
	setText( QObject::tr( "Add %1" )
	                .arg( createCommandString( itemClassName, m_pos ) ) );
	m_canUndo=true;
}


void AddCommand::redo()
{
	m_canUndo=false;
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject * m_parent = dynamic_cast<Report::ItemInterface*>( m_page->itemAt( m_pos ) ) ? dynamic_cast<QObject*>( m_page->itemAt( m_pos ) )
	                : dynamic_cast<Report::PageInterface*>( m_page );
	Report::ItemInterface* itemExample = m_mainWindow->m_reportEngine.findItemByClassName( m_itemClassName );

	Report::ItemInterface *m_item = 0;

	if ( dynamic_cast<Report::ItemInterface*>( m_parent ) )
	{
		if ( dynamic_cast<Report::ItemInterface*>( m_parent )->canContain( itemExample ) )
			m_item = dynamic_cast<Report::ItemInterface*>( itemExample->createInstance( dynamic_cast<QGraphicsItem*>( m_parent ), m_parent ) );
	}
	else
		if ( dynamic_cast<Report::PageInterface*>( m_parent )->canContain( itemExample ) )
			dynamic_cast<Report::PageInterface*>( m_parent )->addItem( m_item = dynamic_cast<Report::ItemInterface*>( itemExample->createInstance( 0, m_parent ) ) );

	if ( m_item )
	{
		m_itemName = Report::ReportEngine::uniqueName( m_item->metaObject()->className(), m_mainWindow->m_report );
		m_item->setObjectName( m_itemName );

		QObject::connect( m_item, SIGNAL( itemSelected( QObject*, QPointF ) ), m_mainWindow, SLOT( itemSelected( QObject*, QPointF ) ) );
		QObject::connect( m_item, SIGNAL( geometryChanged( QObject*, QRectF, QRectF ) ), m_mainWindow, SLOT( itemGeometryChanged( QObject*, QRectF, QRectF ) ) );


		if ( dynamic_cast<Report::BandInterface*>( m_item ) )
			dynamic_cast<Report::BandInterface*>( m_item )->setOrder( INT_MAX );

		QPointF localPos = m_item->mapFromScene( m_pos );
		m_item->setGeometry( QRectF( localPos.x(), localPos.y(), m_item->geometry().width(), m_item->geometry().height() ) );
		m_mainWindow->m_pe->setObject( m_item );
		m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
		m_mainWindow->selectObject( m_item, m_mainWindow->m_objectModel.index( 0, 0 ) );

		m_itemName = m_item->objectName();
		m_canUndo=true;
	}
	else
	{
		m_mainWindow->m_pe->setObject( m_parent );
		m_mainWindow->selectObject( m_parent, m_mainWindow->m_objectModel.index( 0, 0 ) );
	}
}


void AddCommand::undo()
{
	if (!m_canUndo)
		return;
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	Report::ItemInterface *m_item = dynamic_cast<Report::ItemInterface *>( findObject( m_page, m_itemName ) );

	if ( !m_item )
		return;

	QObject * parent = dynamic_cast<QObject*>( m_item )->parent();

	m_page->removeItem( m_item );

	m_item->setParentItem( 0 );
	dynamic_cast<Report::ItemInterface*>( m_item )->removeItem();
	delete dynamic_cast<Report::ItemInterface*>( m_item );
	m_mainWindow->m_lastSelectedObject = parent;
	m_mainWindow->m_pe->setObject( parent );
	m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
	m_mainWindow->selectObject( parent, m_mainWindow->m_objectModel.index( 0, 0 ) );
	m_item = 0;
}

AddDomObject::AddDomObject(Report::PageInterface* page, const QString& parent, const QString& item, QPointF pos, mainWindow* mw) 
{
	m_mainWindow = mw;
	m_pos = pos;
	m_domObject = item;
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );
	m_parentName=parent;
	setText(QObject::tr( "Open item" ));
	m_canUndo=true;
}

void AddDomObject::undo() 
{
	if (!m_canUndo)
		return;

	Report::PageInterface* page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( page );

	QObject *m_parent;

	if ( m_parentName.isNull() || m_parentName==page->objectName())
		m_parent = dynamic_cast<QObject*>( page );
	else
		m_parent = findObject( page, m_parentName );

	Report::ItemInterface *m_item = dynamic_cast<Report::ItemInterface *>( findObject( page, m_itemName ) );



	if ( !m_item || !m_parent )
		return;

	if ( page )
		page->removeItem( m_item );

	m_item->setParentItem( 0 );
	dynamic_cast<Report::ItemInterface*>( m_item )->removeItem();
	delete dynamic_cast<Report::ItemInterface*>( m_item );
	m_mainWindow->m_lastSelectedObject = m_parent;
	m_mainWindow->m_pe->setObject( m_parent );
	m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
	m_mainWindow->selectObject( m_parent, m_mainWindow->m_objectModel.index( 0, 0 ) );
}

void AddDomObject::redo() 
{
	Report::PageInterface* page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( page );

	QObject *m_parent;
	if ( m_parentName.isNull() || m_parentName==page->objectName())
		m_parent = page;
	else
		m_parent = findObject( page, m_parentName );

	Report::ItemInterface *m_item;
	if ( !m_parent )
	{
		m_canUndo=false;
		return;
	}
	QDomDocument doc;
	doc.setContent( m_domObject );
	QObject * obj = m_mainWindow->m_reportEngine.objectFromDom( m_parent, doc.firstChildElement() );

	if ( dynamic_cast<Report::BandInterface*>( obj ) )
	{
		dynamic_cast<Report::BandInterface*>( obj )->setOrder( INT_MAX );
		dynamic_cast<Report::BandInterface*>( obj )->setGeometry( QRectF( 0, 0, dynamic_cast<Report::BandInterface*>( obj )->geometry().width(), dynamic_cast<Report::BandInterface*>( obj )->geometry().height() ) );
	}

	m_item = dynamic_cast<Report::ItemInterface*>( obj );

	if ( dynamic_cast<Report::ItemInterface*>( m_parent ) )
	{
		if ( !dynamic_cast<Report::ItemInterface*>( m_parent )->canContain( m_item ) )
		{
			delete m_item;
			m_item = 0;
		}
	}
	else
		if ( dynamic_cast<Report::PageInterface*>( m_parent )->canContain( m_item ) )
			dynamic_cast<Report::PageInterface*>( m_parent )->addItem( m_item );
		else
		{
			delete m_item;
			m_item = 0;
		}

	if ( m_item )
	{
		QObject::connect( m_item, SIGNAL( itemSelected( QObject *, QPointF ) ), m_mainWindow, SLOT( itemSelected( QObject *, QPointF ) ) );
		QObject::connect( obj, SIGNAL( geometryChanged( QObject*, QRectF, QRectF ) ), m_mainWindow, SLOT( itemGeometryChanged( QObject*, QRectF, QRectF ) ) );
		foreach( QObject * obj, ( dynamic_cast<QObject *>( m_item ) )->children() )
		{
			QObject::connect( obj, SIGNAL( itemSelected( QObject *, QPointF ) ), m_mainWindow, SLOT( itemSelected( QObject *, QPointF ) ) );
			QObject::connect( obj, SIGNAL( geometryChanged( QObject*, QRectF, QRectF ) ), m_mainWindow, SLOT( itemGeometryChanged( QObject*, QRectF, QRectF ) ) );
		}
		if ( dynamic_cast<Report::BandInterface*>( m_item ) )
			dynamic_cast<Report::BandInterface*>( m_item )->setOrder( INT_MAX );
		else
			m_item->setPos(m_pos);
		m_itemName=m_item->objectName();
		m_mainWindow->m_pe->setObject( m_item );
		m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
		m_mainWindow->selectObject( m_item, m_mainWindow->m_objectModel.index( 0, 0 ) );
		setText(QObject::tr("Open item %1(%2)").arg(m_item->objectName()).arg(m_item->metaObject()->className()));
	}
	else
		m_canUndo=false;

}



MoveCommand::MoveCommand( Report::ItemInterface *item, const QPointF &oldPos, mainWindow* mw )
{
	m_mainWindow = mw;
	m_newPos = dynamic_cast<Report::ItemInterface *>( item )->pos();
	m_oldPos = oldPos;
	m_itemName = item->objectName();
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );

	const char * itemClassName = dynamic_cast<Report::ItemInterface*>( item )->metaObject()->className();
	setText( QObject::tr( "Move %1" )
	                .arg( createCommandString( itemClassName, m_newPos ) ) );
}

void MoveCommand::redo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	Report::ItemInterface *m_item = dynamic_cast<Report::ItemInterface *>( findObject( m_page, m_itemName ) );

	if ( !m_item )
		return;

	dynamic_cast<Report::ItemInterface*>( m_item )->setPos( m_newPos );
}

void MoveCommand::undo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	Report::ItemInterface *m_item = dynamic_cast<Report::ItemInterface *>( findObject( m_page, m_itemName ) );

	if ( !m_item )
		return;
	dynamic_cast<Report::ItemInterface*>( m_item )->setPos( m_oldPos );
}


DelCommand::DelCommand( Report::ItemInterface* item, mainWindow* mw )
{
	m_mainWindow = mw;
	m_parentName = dynamic_cast<Report::ItemInterface*>( item->parent() ) ? dynamic_cast<Report::ItemInterface*>( item->parent() )->objectName() : QString();
	m_itemName = item->objectName();
//    m_page = dynamic_cast<Report::PageInterface*>((item)->scene());
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );

	QDomDocument doc;
	doc.appendChild( mw->m_reportEngine.objectProperties(( QObject * )item, &doc ) );
	m_domObject = doc.toString( 0 );

	const char* itemClassName = item->metaObject()->className();
	setText( QObject::tr( "Delete %1" )
	                .arg( createCommandString( itemClassName, item->pos() ) ) );
}


void DelCommand::redo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *m_parent;

	if ( m_parentName.isNull() )
		m_parent = dynamic_cast<QObject*>( m_page );
	else
		m_parent = findObject( m_page, m_parentName );

	Report::ItemInterface *m_item = dynamic_cast<Report::ItemInterface *>( findObject( m_page, m_itemName ) );



	if ( !m_item || !m_parent )
		return;

	if ( m_page )
		m_page->removeItem( m_item );

	m_item->setParentItem( 0 );
	dynamic_cast<Report::ItemInterface*>( m_item )->removeItem();
	delete dynamic_cast<Report::ItemInterface*>( m_item );
	m_mainWindow->m_lastSelectedObject = m_parent;
	m_mainWindow->m_pe->setObject( m_parent );
	m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
	m_mainWindow->selectObject( m_parent, m_mainWindow->m_objectModel.index( 0, 0 ) );
}

void DelCommand::undo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *m_parent;
	if ( m_parentName.isNull() )
		m_parent = m_page;
	else
		m_parent = findObject( m_page, m_parentName );

	Report::ItemInterface *m_item;

	if ( !m_parent )
		return;

	QDomDocument doc;
	doc.setContent( m_domObject );
	QObject * obj = m_mainWindow->m_reportEngine.objectFromDom( m_parent, doc.firstChildElement() );

	if ( dynamic_cast<Report::BandInterface*>( obj ) )
	{
		dynamic_cast<Report::BandInterface*>( obj )->setOrder( INT_MAX );
		dynamic_cast<Report::BandInterface*>( obj )->setGeometry( QRectF( 0, 0, dynamic_cast<Report::BandInterface*>( obj )->geometry().width(), dynamic_cast<Report::BandInterface*>( obj )->geometry().height() ) );
	}

	m_item = dynamic_cast<Report::ItemInterface*>( obj );

	if ( dynamic_cast<Report::ItemInterface*>( m_parent ) )
	{
		if ( !dynamic_cast<Report::ItemInterface*>( m_parent )->canContain( m_item ) )
		{
			delete m_item;
			m_item = 0;
		}
	}
	else
		if ( dynamic_cast<Report::PageInterface*>( m_parent )->canContain( m_item ) )
			dynamic_cast<Report::PageInterface*>( m_parent )->addItem( m_item );
		else
		{
			delete m_item;
			m_item = 0;
		}

	if ( m_item )
	{
		QObject::connect( m_item, SIGNAL( itemSelected( QObject *, QPointF ) ), m_mainWindow, SLOT( itemSelected( QObject *, QPointF ) ) );
		QObject::connect( obj, SIGNAL( geometryChanged( QObject*, QRectF, QRectF ) ), m_mainWindow, SLOT( itemGeometryChanged( QObject*, QRectF, QRectF ) ) );
		foreach( QObject * obj, ( dynamic_cast<QObject *>( m_item ) )->children() )
		{
			QObject::connect( obj, SIGNAL( itemSelected( QObject *, QPointF ) ), m_mainWindow, SLOT( itemSelected( QObject *, QPointF ) ) );
			QObject::connect( obj, SIGNAL( geometryChanged( QObject*, QRectF, QRectF ) ), m_mainWindow, SLOT( itemGeometryChanged( QObject*, QRectF, QRectF ) ) );
		}
		if ( dynamic_cast<Report::BandInterface*>( m_item ) )
			dynamic_cast<Report::BandInterface*>( m_item )->setOrder( INT_MAX );

		m_mainWindow->m_pe->setObject( m_item );
		m_mainWindow->m_objectModel.setRootObject( m_mainWindow->m_report );
		m_mainWindow->selectObject( m_item, m_mainWindow->m_objectModel.index( 0, 0 ) );
	}
}


PropertyChangeCommand::PropertyChangeCommand( QObject * obj, const QString & propertyName, const QVariant & old_value, const QVariant & new_value, mainWindow* mw )
{
	m_mainWindow = mw;
	m_propertyName = propertyName;
	m_oldValue = old_value;
	m_newValue = new_value;
	m_itemName = obj->objectName();
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );
	setText( QObject::tr( "'%1' property '%2'" ).arg( m_itemName ).arg( propertyName ) );
}

void PropertyChangeCommand::redo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *item;

	if ( m_page->objectName() == m_itemName )
		item = m_page;
	else
		item = findObject( m_page, m_itemName );

	if ( !item )
		return;
	if ( m_propertyName == "objectName" )
		m_itemName = m_newValue.toString();
	item->setProperty( qPrintable( m_propertyName ), m_newValue );
}


void PropertyChangeCommand::undo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *item;
	if ( m_page->objectName() == m_itemName )
		item = m_page;
	else
		item = findObject( m_page, m_itemName );

	if ( !item )
		return;

	if ( m_propertyName == "objectName" )
		m_itemName = m_oldValue.toString();
	item->setProperty( qPrintable( m_propertyName ), m_oldValue );
}


bool PropertyChangeCommand::mergeWith( const QUndoCommand *command )
{
	const PropertyChangeCommand *newCommand = static_cast<const PropertyChangeCommand *>( command );
	QString nName = newCommand->m_itemName;
	QString nProperty = newCommand->m_propertyName;
	QString nPageName = newCommand->m_pageName;

	if (( m_itemName != nName || m_propertyName != nProperty || m_pageName != nPageName ) && ( m_propertyName != "objectName" ) )
		return false;

	m_newValue = newCommand->m_newValue;
	if ( m_propertyName == "objectName" )
		m_itemName = m_newValue.toString();
	setText( QObject::tr( "'%1' property '%2'" ).arg( m_itemName ).arg( m_propertyName ) );
	return true;
}


GeometryChangeCommand::GeometryChangeCommand( QObject* obj, QRectF newGeometry, QRectF oldGeometry, mainWindow* mw )
{
	m_mainWindow = mw;
	m_oldGeometry = oldGeometry;
	m_newGeometry = newGeometry;
	m_itemName = obj->objectName();
	m_pageName = mw->m_tw->tabText( mw->m_tw->currentIndex() );
	setText( QObject::tr( "'%1' change size '%2x%3'" ).arg( m_itemName ).arg( newGeometry.height() ).arg( newGeometry.width() ) );
}


void GeometryChangeCommand::redo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *item;

	if ( m_page->objectName() == m_itemName )
		item = m_page;
	else
		item = findObject( m_page, m_itemName );

	if ( !item )
		return;

	if ( dynamic_cast<Report::ItemInterface*>( item ) )
		dynamic_cast<Report::ItemInterface*>( item )->setGeometry( m_newGeometry );
	else
		dynamic_cast<Report::PageInterface*>( item )->setGeometry( m_newGeometry );
}


void GeometryChangeCommand::undo()
{
	Report::PageInterface* m_page = ( Report::PageInterface* )dynamic_cast<QGraphicsView *>( findObjectByTabName( m_mainWindow->m_tw, m_pageName ) )->scene();
	Q_ASSERT( m_page );

	QObject *item;
	if ( m_page->objectName() == m_itemName )
		item = m_page;
	else
		item = findObject( m_page, m_itemName );

	if ( !item )
		return;

	if ( dynamic_cast<Report::ItemInterface*>( item ) )
		dynamic_cast<Report::ItemInterface*>( item )->setGeometry( m_oldGeometry );
	else
		dynamic_cast<Report::PageInterface*>( item )->setGeometry( m_oldGeometry );
}


bool GeometryChangeCommand::mergeWith( const QUndoCommand *command )
{
	const GeometryChangeCommand *newCommand = static_cast<const GeometryChangeCommand *>( command );
	QString nName = newCommand->m_itemName;
	QString nPageName = newCommand->m_pageName;

	if ( m_itemName != nName || m_pageName != nPageName )
		return false;

	m_newGeometry = newCommand->m_newGeometry;

	setText( QObject::tr( "'%1' change size '%2x%3'" ).arg( m_itemName ).arg( m_newGeometry.height() ).arg( m_newGeometry.width() ) );
	return true;
}


NewPageCommand::NewPageCommand( mainWindow * mw )
{
	m_mainWindow = mw;
}


void NewPageCommand::redo()
{
	m_index = m_mainWindow->_createNewPage_();
	setText( QObject::tr( "NewPage \'%1\'" ).arg( m_mainWindow->m_tw->tabText( m_index ) ) );
}

void NewPageCommand::undo()
{
	m_mainWindow->_deletePage_( m_index );
}


RemovePageCommand::RemovePageCommand( mainWindow * mw, int index )
{
	m_mainWindow = mw;
	m_pageName = mw->m_tw->tabText( index );
}

void RemovePageCommand::redo()
{
	int index = findIndexByTabName( m_mainWindow->m_tw, m_pageName );
	m_mainWindow->_deletePage_( index );

// FIXME: need Store page elements

	setText( QObject::tr( "Remove Page \'%1\'" ).arg( m_pageName ) );
}

void RemovePageCommand::undo()
{
	int index = m_mainWindow->_createNewPage_();
	m_mainWindow->m_tw->setTabText( index, m_pageName );
}

/*
ChangePageCommand::ChangePageCommand(mainWindow * mw, int newIndex, int oldIndex)
{
    this->mw = mw;
    this->newName = mw->m_tw->tabText(newIndex);
    this->oldName = mw->m_tw->tabText(oldIndex);
    this->fromStack = false;
    setText(QObject::tr("change page \'%1\'-> \'%2\'").arg(oldName).arg(newName));
}

void ChangePageCommand::redo()
{
    if (fromStack)
    {
        int index = findIndexByTabName(m_tw, newName);
        if (index >= 0)
            mw->m_tw->setCurrentIndex(index);
    }
    else
        fromStack = true;
}

void ChangePageCommand::undo()
{
    int index = findIndexByTabName(m_tw, oldName);
    if (index >= 0)
        mw->m_tw->setCurrentIndex(index);
}

*/

QString createCommandString( Report::ItemInterface  *item, const QPointF &pos )
{
	return QObject::tr( "%1 at (%2, %3)" )
	                .arg( item->metaObject()->className() )
	                .arg( pos.x() ).arg( pos.y() );
}

QString createCommandString( const char* name, const QPointF &pos )
{
	return QObject::tr( "%1 at (%2, %3)" )
	                .arg( name )
	                .arg( pos.x() ).arg( pos.y() );
}

QObject * findObject( Report::PageInterface * page, QString name )
{
	QList<QObject*> list = page->findChildren<QObject *>( name );
	if ( list.isEmpty() )
		return 0;

	return dynamic_cast<Report::ItemInterface *>( page->findChildren<QObject *>( name ).first() );
}


QWidget * findObjectByTabName( QTabWidget * tw, QString tabName )
{
	for ( int i = 0; tw->count(); i++ )
	{
		if ( tw->tabText( i ) == tabName )
			return tw->widget( i );
	}
	return 0;

	/*
	QList<QObject*> list = tw->findChildren<QObject *>(tabName);
	if (list.isEmpty())
	    return 0;

	foreach (QObject* obj, list)
	{
	    if (!dynamic_cast<QGraphicsView *>(obj))
	        continue;
	    return dynamic_cast<QGraphicsView *>(obj);
	}
	return 0;
	*/
}

int findIndexByTabName( QTabWidget * tw, QString tabName )
{
	for ( int i = 0; tw->count(); i++ )
	{
		if ( tw->tabText( i ) == tabName )
			return i;
	}
	return -1;
}