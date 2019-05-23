<?php
session_start();
define('DB_HOST', 'localhost');
define('DB_NAME', 'mails');
define('DB_USER', '');
define('DB_PASSWORD', '');
?>
<html>

<head>
    <title>lolll</title>
    <style>
        .baap {
            width: 50%;
            position: absolute;
            left: 50%;
            margin-left: -25%;
            top: 100px;
        }

        textarea {
            width: 80%;
            margin-bottom: 30px;
            padding: 10px;
        }

        input[type=text] {
            width: 60%;
            margin-bottom: 30px;
        }
    </style>
</head>

<body>
    <div class="baap">
        <form method="POST">
            <label for="email">Enter Email Address: </label><input id="email" type="text" name="email"><br>
            <input type="submit">
        </form>
        <hr>
        <?php
        if (isset($_POST['email']) || isset($_SESSION['email'])) {
            if (isset($_POST['email'])) {
                $_SESSION['email'] = $_POST['email'];
                $_SESSION['email']=str_replace(".","_dot_",$_SESSION['email']);
                $_SESSION['email']=str_replace("@","_at_",$_SESSION['email']);
                echo "<h3> MAILBOX OF ".$_SESSION['email']."</h3><hr>";
            }
            $con = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME) or die("Failed to connect to MySQL: " . mysqli_error());
            if (mysqli_connect_errno()) {
                echo "Failed to connect to MySQL: " . mysqli_connect_error();
            }
            $query = mysqli_query($con, "Select * from ".$_SESSION['email']) or die("Failed to connect to MySQL: " . mysqli_error($con));
            if ($query !== NULL) {
                while ($row = mysqli_fetch_array($query)) {
                    echo "<p>FROM: " . $row[0] . "</p><textarea rows='10'>" . $row[1] . "</textarea><br><hr><br>";
                }
            } else {
                echo "NO MAILS!";
            }
        }
        ?>
    </div>
</body>

</html>
