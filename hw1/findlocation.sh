#!/bin/bash


while getopts n: flag
do
    case "${flag}" in
	n) prefix=${OPTARG};;
    esac
done

if [ -z $@ ]
then
    echo 'Usage: -n "6-digit number"\n'
    exit 1;
fi


if ! [[ $@ =~ ^[0-9]+$ ]];
then
    echo 'Usage: -n "6-digit number"\n'
    exit 1;
fi

temp=$( grep $@ nanpa);
sed -n 's/......//p' <<<"$temp";
 

