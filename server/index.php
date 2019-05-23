<html>
<head>
    <title>lol</title>
</head>
<style>
    body {
        font-family: sArial;
    }

    .baap{
        width: 50%;
        position: absolute;
        left: 50%;
        margin-left: -25%;
        top: 100px;
    }
    .container {
        width: 100%;
        border: solid 1px black;
        padding: 30px;
        box-sizing: border-box;
        text-align: center;
    }

    .click {
        height: 100px;
        width: 100px;
        background-color: red;
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
    #output{
        height: 200px;
        overflow-y: scroll;
        border: solid 1px black;
        padding:20px;
        font:
    }
</style>

<body>
    <div class="baap">
        <div class='container'>
            <label for="from">Sender's Email Address: </label><input id="from" type="text" name="from"><br>
            <label for="to">Receiver's Email Address: </label><input placeholder="seperate multiple recepients by spaces" id="to" type="text" name="to">
            <textarea id="text" rows="10" cols="" placeholder="email body"></textarea><br>
            <button onclick="send()">Send</button>
        </div>
        <h3>Server transaction-</h3>
        <div id='output'>
        </div>
    </div>


    <script>
        function send() {
            var f = document.getElementById('from').value;
            var t = document.getElementById('to').value;
            var text = document.getElementById('text').value;
            var xmlhttp = new XMLHttpRequest();
            xmlhttp.onreadystatechange = function() {
                console.log(this.status);
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById('output').innerHTML = this.responseText;
                    console.log(this.responseText);
                }
            };
            xmlhttp.open("POST", "/smtp/runner.php", true);
            xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xmlhttp.send("t=" + t + "&f=" + f + "&text=" + text);
        }
    </script>

</html>