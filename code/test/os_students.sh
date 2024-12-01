make distclean
make 
#timeout 1 ../build.linux/nachos -e hw3_consoleIO_1 -e hw3_consoleIO_2 -e hw3_consoleIO_3 -e hw3_consoleIO_4 -e
##11/29 
#timeout 1 ../build.linux/nachos -e consoleIO_test1 -e consoleIO_test2
../build.linux/nachos -ep hw3_consoleIO_1 60 -ep hw3_consoleIO_2 70 


# timeout 1 ../build.linux/nachos -e consoleIO_test1
echo "done"
