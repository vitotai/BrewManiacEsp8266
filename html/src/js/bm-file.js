// BM extention, for file manage
BM._listPath="list.php";
BM._ulPath = "upfile.php";
BM._rmUrl = "rm.php";

BM.getFile=function(file,done,fail,always){
    $.ajax({
        method: "GET",
        url: BM._url(file),
    })
    .done(function(msg) {
        if(typeof done == "function") done(msg);
    })
    .fail(function( jqXHR, textStatus){
        if(typeof fail == "function") fail(textStatus);
    })
    .always(function() {
        if(typeof always == "function") always();
    });
};

BM.listDir=function(dir,done,fail,always){
    $.ajax({
        method: "POST",
        data: { dir: dir },
        url: BM._url(BM._listPath),
    })
    .done(function(msg) {
        if(typeof done == "function") done(msg);
    })
    .fail(function( jqXHR, textStatus){
        if(typeof fail == "function") fail(textStatus);
    })
    .always(function() {
        if(typeof always == "function") always();
    });
};

BM.upload=function(fn, data, type, done, fail) {
        var form = new FormData();
        var blob = new Blob([data], { type: type });
        form.append("file", blob, fn);
        var request = new XMLHttpRequest();
        request.open("POST", BM._url(BM._ulPath));
        request.send(form);
        request.onload = function(ev) {
            if (request.status == 200) {
                if (typeof done == "function") done(request.responseText);
            } else {
                if (typeof fail == "function") fail(request, request.statusText );
                else console.log("failed:" + request.statusText );
            }
        };
};

BM.rm=function(file, done, fail) {
    $.ajax({
            url: BM._url(BM._rmUrl),
            type: "POST", // could've used DELETE
            data: { file: file}})
    .done(function(data){
            if (typeof done == "function") done(data);
    }).fail(function(xhr, status) {
            if (typeof fail == "function") fail(status);
    });
};

BM.getBinary=function(m,url,done,fail){
    var xhr = new XMLHttpRequest();
    xhr.open(m,BM._url(url));
    xhr.responseType = 'arraybuffer';
    xhr.onload = function(e) {
        var data = new Uint8Array(this.response);
        done(data);
    };
    xhr.oerror = function(e) {
        if(typeof fail == "function") fail(e);
    };
    xhr.send();

};