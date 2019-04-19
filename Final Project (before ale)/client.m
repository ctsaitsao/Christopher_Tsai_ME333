function client(port)
%   provides a menu for accessing PIC32 motor control functions
%
%   client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf('a: Read current sensor (ADC counts)\nb: Read current sensor (mA)\nc: Read encoder (counts)\nd: Read encoder (deg)\ne: Reset encoder\nf: Set PWM (-100 to 100)\ng: Set current gains\nr: Get mode\nq: Quit\n');
    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
    
    % take the appropriate action
    switch selection
        case 'a'
            adc_counts = fscanf(mySerial, '%d');
            fprintf('The current sensor detects %d ADC counts.\n', adc_counts);
        case 'b'
            mA = fscanf(mySerial, '%f');
            fprintf('The current sensor detects %f mA.\n', mA);            
        case 'c'
            counts = fscanf(mySerial, '%d');
            fprintf('The motor angle is %d counts.\n', counts);
        case 'd'
            deg = fscanf(mySerial, '%d');
            fprintf('The motor angle is %d degrees.\n', deg);
        case 'e'
            counts = fscanf(mySerial, '%d');
            fprintf('The motor angle has been reset to %d counts.\n', counts);
        case 'q'
            has_quit = true;             % exit client
        case 'r'
            mode = fscanf(mySerial, '%d');
            fprintf('The mode index is %d.\n', mode);
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end
