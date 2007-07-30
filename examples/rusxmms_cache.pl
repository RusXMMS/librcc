#! /bin/bash

if [ -z "$1" ]; then
    echo "Please, specify directory with mp3 files (the list.txt should contain language information)"
    echo
    exit
fi
    
list=$1/list.txt
if [ ! -f $list ]; then
    echo "The list.txt with language information is missing in specified directory"
    echo
    exit
fi

rccconf=~/.rcc/xmms.xml
if [ ! -f $rccconf ]; then
    tagreader
    if [ ! -f $rccconf ]; then
	echo "Can't locate RCC configuration"
	echo
	exit
    fi
fi


echo "Setting timeout to 5s"
perl -pi -e 's|<Option name="TIMEOUT">.*</Option>|<Option name="TIMEOUT">5000000</Option>|' $rccconf

echo "Setting leraning mode to LELEARN"
perl -pi -e 's|<Option name="LEARNING_MODE">.*</Option>|<Option name="LEARNING_MODE">RELEARN</Option>|' $rccconf

for filename in $1/*.mp3; do
    name=`basename "$filename"`
    lang=`grep "$name" $list | cut -d ' ' -f 1`

    echo "MP3: $name, Language: $lang"
    perl -pi -e "s|<Language>.*</Language>|<Language>$lang</Language>|" $rccconf

    echo " Recoding..."
    perl -pi -e 's|<Option name="TRANSLATE">.*</Option>|<Option name="TRANSLATE">OFF</Option>|' $rccconf
    tagreader $filename &> /dev/null

    echo " Translating to English..."
    perl -pi -e 's|<Option name="TRANSLATE">.*</Option>|<Option name="TRANSLATE">TO_ENGLISH</Option>|' $rccconf
    tagreader filename &> /dev/null

    echo " Translating..."
    perl -pi -e 's|<Option name="TRANSLATE">.*</Option>|<Option name="TRANSLATE">FULL</Option>|' $rccconf
    tagreader filename &> /dev/null
done

echo "Setting leraning mode to ON"
perl -pi -e 's|<Option name="LEARNING_MODE">.*</Option>|<Option name="LEARNING_MODE">ON</Option>|' $rccconf
