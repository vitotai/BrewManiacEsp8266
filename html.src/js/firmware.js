var Q = function(d) {
    return document.querySelector(d);
};
function show(i) {
    Q("#" +i).style.display = 'block'
}

function hide(i) {
    Q("#" +i).style.display = 'none'
}

function getPara(a) {
    var b = window.location.href;
    a = a.replace(/[\[\]]/g, "\\$&");
    var c = new RegExp("[?&]" + a + "(=([^&#]*)|&|#|$)"),
        results = c.exec(b);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "))
}

function invoke(a) {
    var b = new XMLHttpRequest();
    b.onreadystatechange = function() {
        if (b.readyState == 4) {
            if (b.status == 200) {
                a.success(b.responseText)
            } else {
                if (typeof a["fail"] != "undefined") a.fail(b.status);
                else alert("Unexpected Error.")
            }
        }
    };
    b.ontimeout = function() {
        if (typeof a["timeout"] != "undefined") a.timeout();
        else b.onerror()
    }, b.onerror = function() {
        if (typeof a["fail"] != "undefined") a.fail(-1);
        else alert("Communication Error.")
    };
    b.open(a.m, a.url, true);
    if (typeof a["mime"] != "undefined") b.setRequestHeader("Content-Type", a.mime);
    if (typeof a["data"] != "undefined") {
        b.send(a.data)
    } else b.send()
}

function noSubmit() {
    var a = document.getElementsByTagName("input");
    for (i = 0; i < a.length; i++) {
        a[i].disabled = true
    }
}

function SoftwareUpdater(p) {
    this.pre = p
}
SoftwareUpdater.prototype.loadinfo = function(a, b, h, f) {
    show(this.pre + "initial");
    var t = this;
    f = (f) ? f : false;
    invoke({
        m: "GET",
        url: a,
        success: function(r) {
            hide(t.pre + "initial");
            data = JSON.parse(r);
            if (!f && b == data.version) {
                show(t.pre + "latest")
            } else {
                h(data);
                show(t.pre + "avail")
            }
        },
        fail: function() {
            hide(t.pre + "initial");
            show(t.pre + "error")
        }
    })
};

function countdown(a, b, c) {
    setTimeout(function() {
        location.reload()
    }, a * 1000);
    Q("#" + b).innerHTML = c + "<h2>Wait to reconnect in <span id=COUNTDOWN>" + a + "</span> seconds.</h2>";
    setInterval(function() {
        ele = Q("#COUNTDOWN");
        ele.innerHTML = "" + (parseInt(ele.innerHTML) - 1)
    }, 1000)
}

function fwprogress() {
    invoke({
        m: "GET",
        url: ver_info.fwus,
        timeout: function() {
            Q("#fw_progress").insertAdjacentHTML('beforeend', "...");
            setTimeout(fwprogress, 3000)
        },
        success: function(r) {
            d = JSON.parse(r);
            if (d.finished) {
                if (typeof d["refresh"] != "undefined") {
                    countdown(d.refresh, "fw_progress", "Update Succeeded.")
                } else {
                    Q("#fw_progress").innerHTML = d.result
                }
            } else {
                Q("#fw_progress").insertAdjacentHTML('beforeend', "...");
                setTimeout(fwprogress, 3000)
            }
        }
    })
}

function updatefw() {
    hide("fw_avail");
    show("fw_progress");
    noSubmit();
    invoke({
        m: "POST",
        url: ver_info.fwu,
        mime: "application/x-www-form-urlencoded",
        data: "source=" + encodeURIComponent(fwurl),
        success: function() {
            setTimeout(fwprogress, 3000)
        }
    })
}
var jsfiles;
var processingIndex;

function jsprogress() {
    invoke({
        m: "GET",
        url: ver_info.jsus,
        timeout: function() {
            setTimeout(jsprogress, 3000)
        },
        success: function(r) {
            data = JSON.parse(r);
            if (typeof data["formated"] != "undefined") {
                // checking formating
                if (data.formated) {
                    startProcessFile();
                } else {
                    setTimeout(jsprogress, 5000)
                }
            } else if (data.finished) {
                if (data.error != 0) {
                    alert("error:" + data.msg)
                } else {
                    processingIndex++;
                    updatelist(processingIndex);
                    if (processingIndex == window.targetFileList.length) {
                        invoke({
                            m: "GET",
                            url: "/system-reboot",
                            success: function() {
                                countdown(20, "listdes", "All file updated.")
                            }
                        })
                    } else {
                        processFile(processingIndex)
                    }
                }
            } else {
                setTimeout(jsprogress, 3000)
            }
        }
    })
}

function processFile(i) {
    data = "action=" + window.targetFileList[i].action + "&dst=" + encodeURIComponent(window.targetFileList[i].dst);
    if (window.targetFileList[i].action != "del") data += "&src=" + encodeURIComponent(window.targetFileList[i].src);
    invoke({
        m: "POST",
        url: ver_info.jsu,
        mime: "application/x-www-form-urlencoded",
        data: data,
        success: function() {
            setTimeout(jsprogress, 3000)
        }
    })
}

function formatSpifs() {
    var formating = Q("#listdes");
    invoke({
        m: "GET",
        url: "format-spiffs?update=1",
        success: function(r) {
            setTimeout(jsprogress, 40000);
            window.blinkFormating = setInterval(function() {
                if (formating.style.opacity == 0) formating.style.opacity = 1;
                else formating.style.opacity = 0
            }, 500);
        }
    });
}

function startProcessFile() {
    Q("#listdes").innerHTML = "Updating..";
    Q("#listdes").style.opacity = 1;
    if (window.blinkFormating) clearInterval(window.blinkFormating);
    processingIndex = 0;
    processFile(processingIndex);
    updatelist(0);
}

function updatejs() {
    show("filedetial");
    noSubmit();
    if (window.freshInstall) {
        Q("#listdes").innerHTML = "Formating..";
        formatSpifs();
    } else
        startProcessFile();
}

function showfilelist() {
    if (Q("#filedetial").style.display == "none") show("filedetial");
    else hide("filedetial")
}
var fwurl = "";

function setTargetList(targetlist) {
    // clear old list ,if any
    
    // filter out
    var list=[];
    for(var i=0;i<targetlist.length;i++){
        if(typeof targetlist[i].lang == "undefined")
            list.push(targetlist[i]);
        else if (window.language == targetlist[i].lang)
            list.push(targetlist[i]);
    }

    var oldlist = Q("#filedetial").getElementsByTagName("ul");
    if (oldlist.length > 0) {
        oldlist[0].parentNode.removeChild(oldlist[0]);
    }
    window.targetFileList = list;
    Q("#fileno").innerHTML = "" + list.length;
    var c = document.createElement("ul");
    window.targetFileList.forEach(function(a, i) {
        var b = document.createElement("li");
        b.innerHTML = "[" + a.action + "] " + a.dst;
        c.appendChild(b)
    });
    Q("#filedetial").appendChild(c)
}

function languages(list,lang){

    for (var i = 0; i < list.length; i ++ ) {
        var option = document.createElement( 'option' );
        option.value = option.text = list[i];
        Q("#lang").add(option);
    }
    Q("#lang").value = lang;
    window.language=lang;

    Q("#lang").onchange=function(){
        window.language=Q("#lang").value;
        freshinstallchange();
    };
}

function checkUpdate() {
    var e = (getPara("forced") != null);
    Q("#fw_version").innerHTML = "" + ver_info.fw;
    var f = new SoftwareUpdater("fw_");
    f.loadinfo(ver_info.fwurl + "&opt=" + ver_info.opt, ver_info.fw, function(d) {
        Q("#newversion").innerHTML = "" + d.version;
        Q("#infolink").href = d.url;
        Q("#fsize").innerHTML = d.size;
        fwurl = d.source
    }, e);
    Q("#js_version").innerHTML = "" + ver_info.js;
    var g = new SoftwareUpdater("js_");
    var h = (e) ? "&forced=1" : "";
    g.loadinfo(ver_info.jsurl + ver_info.js + h, ver_info.js, function(d) {
        Q("#jsnewversion").innerHTML = "" + d.version;
        Q("#jsinfolink").href = d.url;
        hide("filedetial");
        jsfiles = d;
        languages(jsfiles.languages,jsfiles.default_language);
        setTargetList(jsfiles.list);
    }, e)
}

function updatelist(d) {
    var e = document.createElement("ul");
    window.targetFileList.forEach(function(a, i) {
        var b = document.createElement("li");
        if (i == d) {
            b.innerHTML = "[Processing]" + a.dst;
            var c = b;
            setInterval(function() {
                if (c.style.opacity == 0) c.style.opacity = 1;
                else c.style.opacity = 0
            }, 500)
        } else if (i < d) b.innerHTML = "[DONE] " + a.dst;
        else b.innerHTML = "[wait] " + a.dst;
        e.appendChild(b)
    });
    var f = Q("#filedetial");
    var g = f.getElementsByTagName("ul")[0];
    f.removeChild(g);
    f.appendChild(e)
}

function freshinstallchange() {
    //console.log("fresh installed:" + ckbox.target.checked);
    window.freshInstall = Q("#freshinstall").checked;
    if (window.freshInstall) {
        setTargetList(jsfiles.fl);
    } else {
        setTargetList(jsfiles.list);
    }
}
var ver_info = {};
function sysinfo(info) {
    //
    Q("#flash-id").innerText="0x"+ info.fid.toString(16);
    var vendor = parseInt(info.fid) & 0xFF;
    Q("#flash-vendor").innerText="0x"+ vendor.toString(16);
    Q("#real-flash-size").innerText=info.rsize;
    Q("#specified-flash-size").innerText=info.ssize;
    Q("#fs-size").innerText=info.fs;
    var str="";
    for(var i=0;i<12;i+=2){
        str += ((str == "")? "":":") + info.mac.substring(i,i+2);
    }
    Q("#mac-address").innerText=str;
}
function init() {
    Q("#freshinstall").onclick = freshinstallchange;
    invoke({
        m: "GET",
        url: "version.php",
        success: function(r) {
            ver_info = JSON.parse(r);
            checkUpdate();
            sysinfo(ver_info.system);
        }
    })
}