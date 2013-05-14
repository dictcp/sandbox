<?php
//
// Convert from virt-top CSV to a human-readable summary-like CSV
// by Dick Tang
//

$fp=fopen($argv[1],"r");

fgetcsv($fp, 0, ',' );
printf("%s\t%s\t%s\t%s\t%s\t%s\n", "VM", "CPU", "BLK_R", "BLK_W", "NET_RX", "NET_TX");
while (FALSE != ($data=fgetcsv($fp, 0, ',' )) ) {
  $inst=0;
  $cpu=0;
  $block_r=0;
  $block_w=0;
  $net_rx=0;
  $net_tx=0;

  $inst=$data[4];
  for ($i=18;$i<18+10*$inst;$i+=10) {
    $cpu+=$data[$i+2]>0?$data[$i+2]:0;
    $block_r+=$data[$i+6]>0?$data[$i+6]:0;
    $block_w+=$data[$i+7]>0?$data[$i+7]:0;
    $net_rx+=$data[$i+8]>0?$data[$i+8]:0;
    $net_tx+=$data[$i+9]>0?$data[$i+9]:0;
  }
  $cpu=$data[13];

  printf("%d\t%f\t%d\t%d\t%d\t%d\n", $inst, $cpu, $block_r, $block_w, $net_rx, $net_tx);
}

