DFM pattern capture application
===============================

I would like to develop a DFM pattern capture application. This application will be a command-line application. It will read
 an input layout file (either OASIS or GDSII file) and capture multi-layer patterns from the file, and store the patterns in a database.

 The command-line application will take as input the following arguments:

 1) -layout_file <layout_file>    : description: <layout_file> is the path to the layout file to capture the patterns from
 2) -mask_layer_number <mask_layer_number>    : description: <mask_layer_number> is the layer number inside the layout file for the mask layer
 3) -input_layers_numbers <input_layers_numbers>     : description: <input_layers_numbers> is a space-separated list of the input layers numbers to capture the patterns from
 4) -db_name <db_name> : The name of a PostgreSQL database where the captured patterns will be stored

 After parsing the input arguments, the application will read the mask layer and the input layers from the layout file, 
 then for each polygon on the mask layer, it will AND the polygon on the mask layer with the geoemtries each input layer.

 THe result of each AND operation will be a layer of the multi-layer captured pattern. Then it will store the constructed 
 pattern inside the PostgreSQL database.
 
===========================================
 The command shall be implemented in C++
===========================================
