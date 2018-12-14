var Logs = {
    url: "logs.php",
    dl: "dl",
    init: function() {
        var t = this;
        this.row = $("#loglist").find("tr:nth-of-type(2)");
        this.row.remove();
        $("#listfile").button().button("option", "icons", { primary: "ui-icon-arrowrefresh-1-e" }).click(function() { t.getlist(); });
        $("#vchart").hide();
        t.chart = new BrewChart("vchart-canvas");
    },
    view: function(name, date) {
        //console.log("view chart:"+name);
        $("#vchart").show();
        $("#viewlogname").text(name + "@" + date.toLocaleString());
        var t = this;
        t.chart.clear();

        t.chart.setCelius(BM.celius);

        var xhr = new XMLHttpRequest();
        xhr.open('GET', t.url + "?" + t.dl + "=" + name);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function(e) {
            var data = new Uint8Array(this.response);
            if (data.length == 0) {
                //console.log("zero content");
                return;
            }
            t.chart.realtime = true;
            t.chart.process(data);
        };
        xhr.oerror = function(e) {
            console.log("error");
        };
        xhr.send();
    },
    process: function(json) {
        var tb = $("#loglist").find("tbody");
        var row = this.row;
        var t = this;
        $.each(json, function(i, fi) {
            var name = ('000' + fi.f).slice(-4);
            var date = new Date(fi.t * 1000);
            //console.log("file:"+ name + " time:"+ date);
            var nr = row.clone();
            nr.find(".logid").text(name);
            nr.find(".logdate").text(date.toLocaleString());
            nr.find(".dlbutton").button().button("option", "icons", { primary: "ui-icon-disk" }).click(function() {
                window.open(t.url + "?" + t.dl + "=" + name);
            });
            nr.find(".viewbutton").button().button("option", "icons", { primary: "ui-icon-circle-zoomout" }).click(function() {
                t.view(name, date);
            });
            tb.append(nr);
        });
    },
    getlist: function() {
        var t = this;
        $.ajax({
            url: t.url,
            type: "GET",
            dataType: "json",
            success: function(json) {
                $("#loglist tr:gt(0)").remove();
                t.process(json);
            },
            error: function(xhr, status, errorThrown) {
                console.log("Error:" + errorThrown);
            }
        });
    }
};