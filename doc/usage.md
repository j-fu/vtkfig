Usage information
=================

## General philosophy

All data are stored in instances of vtkfig::DataSet which in fact just wrap [vtkDataSet](http://www.vtk.org/doc/nightly/html/classvtkDataSet.html) instances describing a grid with all its attached scalar and vector functions.

Create such a data set e.g. via
~~~
vtkfig::DataSet data;
data.SetRectilinearGrid(x,y);
data.SetPointScalar(v ,"v");
~~~

A particular visualization of a data set is done
via a subclass of vtkfig::Figure.
~~~
vtkfig::ScalarView contour;
contour.SetData(data,"v");
~~~

Just note that a data set can have multiple functions attached, which can be shown in different figures, which share one instance of grid information.


To be seen on the screen, put the figure into an instance of vtkfig::Frame:

~~~
vtkfig::Frame frame;
frame.AddFigure(contour);
frame.Interact()
~~~

A vtkfig::Frame can have subframes (similar to [subplots in matplotlib](https://matplotlib.org/examples/pylab_examples/subplot_demo.html)).


If instead of blocking interaction you want to render data evolving during time, you can do in a loop
~~~
while(...)
{
 ... calculate new values for v ...

  // update data in data set 
  data.SetPointScalar(v ,"v");

  // non-blocking
  frame.Show()
}
~~~

## Smart pointer idiom

In many contexts,  one would like to use smart pointers instead of references to class objects. Therefore all API functions which pass vtkfig class objects have a reference and a smart pointer version. The same example as above then rewrites as follows:

Create such a DataSet:
~~~
auto data=vtkfig::DataSet::New();
data->SetRectilinearGrid(x,y);
data->SetPointScalar(v ,"v");
~~~

Create a  Figure and add DataSet
~~~
auto contour=vtkfig::ScalarView::New();
contour->SetData(data,"v");
~~~
Create Frame and add Figure
~~~
auto frame=vtkfig::Frame::New();
frame->AddFigure(contour);
frame->Interact()
~~~

The `::New()` methods are static class methods which return  smart pointers (`std::shared_ptr`) to objects.




## Keyboard and mouse interaction

Currently, there  is no proper GUI,  and due to lack  of resources, no plan  to   develop  one.   Nevertheless,  the   visualization  can  be influenced interactively.  Besides of  the standard  mouse interaction from  vtk,   a  number   of  keyboard   commands  is   available,  see vtkfig::Frame::KeyboardHelp.

## Environment variables


| Variable             | Default | Semantics                                         |
|----------------------|---------|---------------------------------------------------|
| VTKFIG_PORT_NUMBER   |   35000 | Port number passed from vtkfig-exec to client     |
| VTKFIG_WAIT_SECONDS  |      10 | Waiting time before abandoning connection attempt |
| VTKFIG_DEBUG         |       0 | Toggle debug output                               |
| VTKFIG_MULTITHREADED |       1 | Render from extra thread                          |
| VTKFIG_DOUBLEBUFFER  |       1 | Use double buffering.                             |



## Server-client  communication

The file  src/vtkfig-exec.cxx builds to an  experimental visualization client  for server-client  communication. Most  examples are  equipped with this feature.  Here is how to  use it (we assume working from the ``build``   directory,   ``port``   is  an   optional   port   number, ``vtkfigpath``  is the  path to  the  vtkfig source  directory on  the remote host ):

   - Run example locally

~~~
$ examples/example-surf2d
~~~

   - Run example in client server mode on localhost

~~~
$ src/vtkfig-exec -p port  localhost examples/example-surf2d
~~~


   - Run example on remote host ``remote``

~~~
$ src/vtkfig-exec -p port -ssh  remote vtkfigpath/build/examples/example-surf2d
~~~

   - Run example on remote host ``remote`` with ssh tunnel via gateway  ``gate``

~~~
$ src/vtkfig-exec -p port -via gate -ssh  remote vtkfigpath/build/examples/example-surf2d
~~~
