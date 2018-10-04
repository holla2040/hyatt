#!/bin/bash
if [ -z "$name" ] 
then
    name=hyatt
fi


cd data
for file in `ls -A1` 
do 
    echo ${file} to ${name}.local
    curl -F "file=@$PWD/${file}" ${name}.local/edit
done
