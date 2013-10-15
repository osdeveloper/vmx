Real VMX Operating System  
   
Starting from scratch with the operating system  
   
Setup enviroment:  
export VMX_BASE=/vmx-source-path  
cd $VMX_BASE  
   
Build instructions:  
cd src  
make CPU=PENTIUM2  
cd build/i386  
make  
   
Installation instructions:  
sudo cp vmx /boot   
   
This only has to be done once!  
sudo cat >> vi /etc/grub.d/40_custom   
menuentry 'Real VMX' {  
    insmod bsd  
    set root=(hd0,n)	# Where n is the partition containing /boot  
    knetbsd /boot/vmx  
}  
sudo update-grub  
   
Reboot and;  
Select Real VMX at the boot menu  

