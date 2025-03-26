<?php

//
// Requirements: Fairly recent version of PHP with DIO extension installed and enabled
//

$help=<<<EOS
*
* Usage: php buzzfeed.php -d DEVICE -b BAUDRATE -l DATABITS -s STOPBITS -p PARITY -f FILENAME -i STRING
*
*   DEVICE on Windows is usually COMx; on linux it's usually /dev/ttyUSBx or /dev/ttyACMx. Default is COM1 for Windows or /dev/ttyACM0 otherwise.
*   BAUDRATE defaults to 115200.
*   DATABITS defaults to 8.
*   STOPBITS defaults to 1.
*   PARITY should be one of [N, O, E] for None, Odd, or Even. Defaults to N.
*
*   At least one of either -f or -i must be specified as a data source.
*   -f FILENAME will read data from the specified filename.
*   -i STRING will take data directly from the specified STRING. If STRING contains spaces, it should be enclosed in quotes.
*

EOS;

$windows = (strtoupper(substr(PHP_OS, 0, 3))=='WIN');
$device = ($windows?'COM1':'/dev/ttyACM0');
$baud = 115200;
$bits = 8;
$stop = 1;
$parity = 'N';

if (!extension_loaded('dio')) die("PHP Direct IO (DIO) not installed\n");

$parcodes=array('N', 'O', 'E');
$filename=$immediate=''; $error=null;
$opts=getopt('d:b:l:s:p:f:i:');
if (isset($opts['d'])) $device=$opts['d'];
if (isset($opts['b'])) intval($baud=$opts['b']);
if (isset($opts['l'])) intval($bits=$opts['l']);
if (isset($opts['s'])) intval($stop=$opts['s']);
if (isset($opts['p'])) { $parity=strtoupper($opts['p']); if (!in_array($parity, $parcodes)) $error=true; }
if (isset($opts['f'])) $filename=$opts['f'];
if (isset($opts['i'])) $immediate=strtoupper(preg_replace('/\s+/', '', $opts['i']));

if ($error || (!strlen($filename) && !strlen($immediate))) die($help);

set_error_handler("error_handler");
try {
    if ($windows) {
        exec("mode {$device} baud={$baud} data={$bits} stop={$stop} parity={$parity} to=on xon=off octs=off odsr=off dtr=off rts=off");
        $serial = dio_open($device, O_WRONLY);
    } else {
        $out=null; $ret=0; @exec("stty -F {$device} -a 2>/dev/null", $out, $ret);
        if ($ret) $error="stty unable to access port settings";
        else {
            exec("stty -F {$device} -hup clocal -crtscts -ixon -ixoff");
            $hup=array_filter($out, function($val) { return strpos($val, '-hup')!==false; });
            if (!count($hup)) { echo "Waiting for reset..."; sleep(3); echo "done.\n"; }
            $serial = dio_open($device, O_RDWR | O_NOCTTY | O_NONBLOCK);
            dio_fcntl($serial, F_SETFL, O_SYNC);
            dio_tcsetattr($serial, array('baud'=>$baud, 'bits'=>$bits, 'stop'=>$stop, 'parity'=>array_search($parity, $parcodes)));
        }
    }
} catch (Exception $e) {
    $error=$e->getMessage();
}
if ($error) {
    echo "Could not open serial device {$device} -- incorrect device, or cable unplugged?\n";
    // You can uncomment the following line for a little more detail, but it probably won't tell you anything more helpful
    //echo $error, "\n";
    exit(1);
}

$databuff=[];
if (strlen($immediate)) {
    $databuff=str_split($immediate);
    process_buffer();
}
if (strlen($filename)) {
    $file=fopen($filename, 'r') or die("Unable to open file $filename\n");
    while (true) {
        $char=fread($file, 1);
        if ($char===false || $char===0 || !strlen($char)) break;
        if ($char==';') { fgets($file); continue; }
        if (ctype_space($char)) continue;
        $databuff[]=strtoupper($char);
        if (count($databuff)==50) process_buffer();
    }
    fclose($file);
}
$x=count($databuff);
if ($x && ($x%2)==0) process_buffer();
dio_close($serial);
exit(0);

function process_buffer() {
    global $databuff;
    static $datamode=0;
    $outbytes=[];
    $wait=0;
    for ($x=0; $x<count($databuff); $x+=2) {
        $ntime=hrtime(true);
        if ($databuff[$x]=='W') {
            send_bytes($outbytes); $outbytes=[];
            $datamode=2;
            continue;
        }
        $value=hexdec($databuff[$x].$databuff[$x+1]);
        if ($datamode==2) {
            $wait=$value;
            $datamode=1;
        }
        else if ($datamode==1) {
            $wait+=$value*256;
            $ntime+=$wait*1000000;
            $datamode=0;
            while (hrtime(true)<$ntime) {}
        }
        else $outbytes[]=chr($value);
    }
    send_bytes($outbytes);
    $databuff=[];
}

function send_bytes($bytes) {
    global $serial;
    $len=count($bytes);
    if (!count($bytes)) return;
    dio_write($serial, chr(90).chr($len).implode($bytes), $len+2);
}

function error_handler($errno, $errstr, $errfile, $errline ) {
    throw new ErrorException($errstr, $errno, 0, $errfile, $errline);
}
?>
