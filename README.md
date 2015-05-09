# autofocus

This project is gPhoto2 connection to OpenCV prove of concept.

It's simple implementation of autofocus based of edges detection.

It was tested only with Nikon DSLR. (Nikon D90).
But shall work on all Nikon DSLRs, and with little effort with other devices.

For OpenCV extension go to my OpenCV fork on github:
[OpenCV with extension](https://github.com/dobrypd/opencv/tree/libgphoto2-connection "OpenCV With gPhoto2 extension")

For detailed gPhoto2 information go to:
[gPhoto2 project](http://gphoto.sourceforge.net/ "gPhoto project site")


The evaluation function is really easy (fraction of edges in frame).
But it is good enough for this purpose. I've tested this function with 6 different scenes.
The results are in plot:
![FunctionEvaluation](https://raw.githubusercontent.com/dobrypd/autofocus/master/evaluation/NikonD90/NikonD90-50mm1.4G-evaluation.svg)

Links to captured scenes:
 * [plant](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-plant.mp4)
 * [tee](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-tee.mp4)
 * [texture](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-texture.mp4)
 * [texture_too_close](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-texture_too_close.mp4)
 * [wall](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-wall.mp4)
 * [window](http://dobrowolski.net.pl/autofocus/NikonD90-50mm1.4G-window.mp4)

