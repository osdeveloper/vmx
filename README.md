Real VMX Operating System  
   
Starting from scratch with the operating system  
   
Build instructions:  
cd vmx/src/build/i386  
make  
   
Installation instructions:  
sudo cp vmx /boot   
sudo cat >> vi /etc/grub.d/40_custom   
menuentry 'Real VMX' {  
    insmod bsd  
    set root=(hd0,n)	# Where n is the partition containing /boot  
    knetbsd /boot/vmx  
}  
sudo update-grub  
   
Select Real VMX at the boot menu  

