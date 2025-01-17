======================
Property output report
======================

The property output report is a JSON-formatted file with the :term:`channel` output results of the
simulation, defined by the groups set up using **IndividualProperties** in the demographics file.
See :ref:`demo-properties` for more information. For example, it allows you to compare disease deaths
for people in the high risk group versus the low risk group. If you want to aggregate the data, you
must create a script for aggregating information. The file name is PropertyReport.json.

To generate the property report, set the **Enable_Property_Output** configuration
parameter to 1.

The file contains a header and a channels section.

Header
======

The header section contains the following parameters.

.. csv-table::
   :header: Parameter, Data type, Description
   :widths: 10,5,20

   DateTime, string, The time stamp indicating when the report was generated.
   DTK_Version, string, The version of |EMOD_s| used.
   Report_Type, string, The type of output report.
   Report_Version, string, The format version of the report.
   Start_Time, integer, The time noted in days when the simulation begins.
   Simulation_Timestep, integer, The number of days in each time step.
   Timesteps, integer, The number of time steps in this simulation.
   Channels, integer, The number of channels in the simulation.


Channels
========

The channels section contains the following parameters.

.. csv-table::
   :header: Parameter, Data type, Description
   :widths: 7,5,30

   <Channel_Title>, string, "The title of the particular property channel. The channel titles use the following conventions: for a single property, <channel type>:<property>:<value>, and for multiple properties, <channel type>:<property 1>:<value>,<property 2>:<value>. For example, Infected:Accessibility:Easy or New Infections:Accessibility:Difficult,Risk:High."
   Units, string, The units used for this channel.
   Data, array, A list of the channel data at each time step.


Example
=======

The following is a sample of a PropertyReport.json file.

.. literalinclude:: json/report-property.json
   :language: json