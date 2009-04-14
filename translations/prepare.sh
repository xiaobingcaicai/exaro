#!/bin/sh

PRO_FILE="translations.pro"

echo "TEMPLATE = app" > $PRO_FILE

echo "TRANSLATIONS += exaro_ro_RO.ts exaro_ru_RU.ts exaro_ru_UA.ts exaro_it_IT.ts" >> $PRO_FILE

for x in `find ../ -name *.cpp|grep -v "moc_"|grep -v "qrc_"`; do
    echo "SOURCES += $x" >> $PRO_FILE
done;

#echo "\n\n" >> $PRO_FILE

for x in `find ../ -name *.ui`; do
    echo "FORMS += $x" >> $PRO_FILE
done;
     
lupdate $PRO_FILE

