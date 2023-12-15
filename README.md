# SUSU_Httpd
A simple C++11 http CGI server

This project references the design of the tinyhttpd

https://github.com/larryhe/tinyhttpd

I hope to create a httpd for myself,so I begin to build SUSU_Httpd,just for fun.

#how to use

1.
	use   make  to get susu_httpd,the binary name is example. 
2.
	set root_path in susu_httpd.conf
3.
	move the get.lua to the root_path

4.
	you can run the example.but you have no html、css、image.so the server is useless.

5.
	add some html、css、images.

Tips.
	because susu_httpd only have get.lua,so susu_httpd can only process GET request.
	but it will support more function in the future.