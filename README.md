# SUSU_Httpd
A simple C++11 http CGI server

This project references the design of the tinyhttpd

https://github.com/larryhe/tinyhttpd

I hope to create a httpd for myself,so I begin to build SUSU_Httpd,just for fun.

# how to use this httpd

1.build a demo

	make demo
	
2.run the demo

	cd ./demo
	
	chmod 755 ./run.sh
	
	./run.sh
	
3.check your address and port.

	
4.if your address is 192.168.0.103 , and susu_httpd is listening port 12345 , you can use 
 	
  	192.168.0.103:12345
   
   in your WebBroser, so you can see the demo web. like this
     
<img width="584" alt="1702686947846" src="https://github.com/ZhengXuan-Xie/SUSU_Httpd/assets/121448413/8f221472-3eac-4618-8b69-fe78fcc41b25">

	

5.if you had run the demo build , you can use this code

	make httpd_binary
