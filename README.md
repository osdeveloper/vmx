Real VMX  - Realtime Operating System.  
Starting from scrarch based on: http://sourceforge.net/projects/vmx/  
   
Setup enviroment:  
export VMX_BASE=/vmx-source-path  
   
Build instructions:  
cd $VMX_BASE  
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

