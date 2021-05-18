Z-stream
========

.. note::

    Based on the work of `Jonathan de Halleux <https://www.codeproject.com/Members/Jonathan-de-Halleux>`__,
    published on CodeProject
    `here <http://www.codeproject.com/Articles/4457/zipstream-bzip-stream-iostream-wrappers-for-the-zl>`__.

STL iostream implementation using the library zlib.
This means that you can easily manipulate zipped
streams like any other STL ostream/istream.

To give you an idea, consider following snippet
that prints "Hello World":

.. code-block:: cpp

    ostringstream output_buffer;
    // writing data
    output_buffer << "Hello world" << endl ;

Now, the same snippet but with zipped output using zlib:

.. code-block:: cpp

    // zip_ostream uses output_buffer as output buffer :)
    ozstream zipper( output_buffer );

    // writing data as usual
    zipper << "Hello world" << endl;

Or, to create gzipped files:

.. code-block:: cpp

    ofstream file("hello_world.txt.gz");
    ogzstream gzfile(file);
    gzfile << "Hello world " << endl;

.. note:: From original documentation:

    As you can see adding zipped buffers into your existing applications
    is quite straightforward.
    To summarize, let's see some quick facts about zstream:

    - STL compliant,
    - any-stream-to-any-stream support,
    - char, wchar_t support,
    - fining tuning of compression properties,
    - support custom allocators
