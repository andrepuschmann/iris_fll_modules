# Iris_FLL_Modules Project

Iris is a software architecture for building highly reconfigurable radio 
networks using a component-based design. 
The Iris_FLL_Modules package contains a number of PHY and Stack components 
as well as controllers that are not part of the official Iris_Modules package. 
In particular, it contains GdtpComponent, a wrapper for using libgdtp 
from within Iris, SoftCsma, a software implementation of a CSMA-based MAC 
protocol, and FpgaCsma, a CSMA-MAC implemented inside the FPGA of the USRP N210. 

## Description

The Iris architecture offers support for all layers of the network stack
and provides a platform for the development of not only reconfigurable 
point-to-point radio links but complete networks of reconfigurable radios. 
Individual radios are described using an XML document. This lists the 
components which comprise the radio, gives the values to be used for 
their parameters and describes the connections between them.

Iris was originally developed by CTVR, The Telecommunications Research 
Centre, based at University of Dublin, Trinity College. 
In 2013, it was released under the LGPL v3 license and is currently 
managed by Software Radio Systems (http://www.softwareradiosystems.com).

## Getting Started

The installation guide can be found [here](http://www.hostedredmine.com/projects/iris_software_radio)

## Requirements

Required:
* Iris_Core (Use the **next** branch from my [iris_core](https://github.com/andrepuschmann/iris_core) repo )

Optional:
* Iris_Modules + Qwt for the graphical components
* UHD for SimpleTdmaTagger
* libgdtp for GdtpComponent (will be build through OSPECORR)
* StateBuilderCpp for the fllcontroller
   * Get and install it from [here](http://www.stateforge.com/Help/StateBuilderCpp/install-statebuildercpp-linux.aspx)


## Where To Get Help

* Redmine: http://www.hostedredmine.com/projects/iris_software_radio
* Iris-discuss mailing list: http://www.softwareradiosystems.com/mailman/listinfo/iris-discuss
* Iris blog: http://irissoftwareradio.wordpress.com

## License

Iris is licensed under LGPL v3.
The Iris FLL modules are licensed under GPL v3. 
For more information see LICENSE.
