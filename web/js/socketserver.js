var wSocket = new WebSocket('ws://localhost:1234');


function PrintGraph() {



}



wSocket.onopen = function(event) {
    alert('Connected');
}

wSocket.onclose = function(event) {
    alert('Finished');
}

wSocket.onmessage = function(event) {

    var Message;
    var Msg;
    var Address;
    var Value;

    var table = document.getElementById("myTable");

    var Adata;
    Message = event.data.split('\n');

    var row;
    var cell1;
    var cell2;

    Adata = [];

    for (i = 0; i < table.rows.length; i++) {
        Msg = Message[i].split(',');

        row = table.rows[i];
        cell1 = row.cells[0];
        cell2 = row.cells[1];
        cell1.innerHTML = Msg[0];
        cell2.innerHTML = Msg[1];
        cell2.contentEditable='true';

        Address = cell1.innerHTML.split(' ');
        Value = cell2.innerHTML.split(' ');

        Adata.push([Address[1], Value[2]]);
    }


    print(Adata);


    var z = g.getValue(1, 1);


}

wSocket.onerror = function(event) {
    alert('This is a bug');
}

function Read() {
//    var txt = document.getElementById('txt');
//    var val = txt.value;
    wSocket.send('Read');
}

function Write() {

    var table = document.getElementById("myTable");
    var Adress;
    var Value;

    var Message = '';


    for (var r = 0, n = table.rows.length; r < n; r++) {
        for (var c = 0, m = table.rows[r].cells.length; c < m; c++) {
            Message  =  Message + table.rows[r].cells[c].innerHTML + ',';
        }
        Message = Message + '\n';
     }

    wSocket.send(Message);
}
