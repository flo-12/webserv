<!DOCTYPE html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RPN is fun.</title>
</head>
<body style="background-color: #dfd2aa; font-family: Roboto, sans-serif;">
    <div style="display: flex; justify-content: center;
    text-align: center; align-items: flex-end; height: 30vh;">
    </div>
    <div style="display: flex; justify-content: center; 
    flex-direction: column; align-items: center; 
    height: 20vh; margin: 0;">
        <h1 style="margin-top: auto;">PHP CGI RPN</h1>
        <form action="RPN.php" method="post" style="margin-top: -7px;">
            <label for="numberInput">Enter numbers:</label>
            <input name="number" required>
            <button type="submit">Calculate</button>
        </form>
    
        <?php
            $input = file_get_contents('php://stdin');
            #echo "Request: " . htmlspecialchars($input);
            $output = shell_exec("./cgi-bin/RPN $input");
        ?>

    <div style="border: 1px solid #000; border-radius: 3px; 
    border-style: dashed; padding: 10px; margin-top: 20px;
    width: 358px; white-space: pre-wrap; font-family: inherit;">
        Result: <?php echo htmlspecialchars($output); ?>
    </div>

    </div>
</body>
</html>