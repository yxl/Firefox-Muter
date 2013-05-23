<xml!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    </head>
<body>
<?php
    // Get the feedback xml content
    $text = file_get_contents('php://input');

    // Compose an email with the feedback content
    $to = "xyuan@mozilla.com";  
    $subject = "[muter feedback]";
    $from = "admin@yxl.name";
    $headers = "";
    $headers .= "MIME-Version: 1.0\r\n";
    $headers .= "Content-type: text/html; charset=utf-8"."\r\n";
    $headers .= "From: ".$from."\r\n";
    $subject = "=?UTF-8?B?".base64_encode($subject)."?=";
    $date = date("Y-M-j h:i:s A");
    $body = $text;
    
    // Send the email 
    if($text) {
      if(@mail($to,$subject,$body,$headers)){
        echo("Succeeded!");
      }else{
        echo("Failed!");
      }
    }
?>
</body>
</html>

