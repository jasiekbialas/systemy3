
umount /root/nowy
mount /dev/c0d1 /root/nowy
cd ../nowy
rm -r *
touch KEY
touch plik

printf '\x2A' > ./KEY
echo 'ALA MA KOTA' > ./plik
if [ $? -ne "0" ] 
then 
    echo ERROR 1.1
fi
cat ./plik | { 
    read test

    if [ "$test" != "ALA MA KOTA" ]
    then 
        echo ERROR 1.2
    fi
 }

cat ./KEY

if [ $? -ne "1" ] 
then 
    echo ERROR 1.3
fi

# cat: ./KEY: Operation not permitted

printf '\x0' > ./KEY
if [ $? -ne "0" ] 
then 
    echo ERROR 1.4
fi

cat ./plik | { 
    read test
    if [ "$test" != "kvkJwkJuy~k4" ]
    then 
        echo ERROR 1.5
    fi
 }

rm -r *
cd ..
umount /root/nowy
mount /dev/c0d1 /root/nowy
cd nowy

touch KEY
touch plik

echo 'ala ma kota' | tee ./plik

if [ $? -ne "1" ] 
then 
    echo ERROR 2.1
fi

cat ./plik

if [ $? -ne "1" ] 
then 
    echo ERROR 2.2
fi

printf '\x2A' > ./KEY

if [ $? -ne "0" ] 
then 
    echo ERROR 2.3
fi

echo 'ala ma kota' | tee ./plik

if [ $? -ne "0" ] 
then 
    echo ERROR 2.4
fi

cat ./plik | { 
    read test
    if [ "$test" != "ala ma kota" ]
    then 
        echo ERROR 2.5
    fi
 }


rm -r *
cd ..
umount /root/nowy
mount /dev/c0d1 /root/nowy
cd nowy

touch KEY
touch plik

touch ./NOT_ENCRYPTED
if [ $? -ne "0" ] 
then 
    echo ERROR 3.1
fi

echo 'ala ma kota' | tee ./plik
if [ $? -ne "0" ] 
then 
    echo ERROR 3.2
fi

cat ./plik | { 
    read test
    if [ "$test" != "ala ma kota" ]
    then 
        echo ERROR 3.3
    fi
 }

printf '\x2A' | tee ./KEY

if [ $? -ne "1" ] 
then 
    echo 3 ERROR 3.4
fi

# tee: ./KEY: Operation not permitted

rm ./NOT_ENCRYPTED

printf '\x2A' | tee ./KEY
if [ $? -ne "0" ] 
then 
    echo ERROR 3.5
fi

cat ./plik | { 
    read test
    if [ "$test" != "7B7�C7�AEJ7�" ]
    then 
        echo ERROR 3.6
    fi
 }
