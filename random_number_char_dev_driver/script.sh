sudo rmmod random_number_driver
sudo insmod random_number_driver.ko
sudo chmod 666 /dev/random_number_char_dev
sudo cat /dev/random_number_char_dev