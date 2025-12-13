oss << R"EOF(

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <style>
    text {
      font-family: monospace;
    }
    html, body {
      margin: 0;
      padding: 0;
    }
    body{
      background-color: #DDDDDD;
    }
    .hide-cursor {
      cursor: none;
    }
    #svgChart, #svgCursor, #svgSnap {
      position: absolute;
      top: 0;
      left: 0;
      overflow: visible;
    }
    #svgChart {
      transform: translateZ(0);
    }
    @media (prefers-color-scheme: dark) {
      body {
        background-color: #112222;
      }
    }
  </style>
)EOF";
