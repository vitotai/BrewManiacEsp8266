var BrewLogs = {
    url: "logs.php",
    dl: "dl",
    init: function(bm) {
        var t = this;
        t.bm=bm;
        this.row = $("#loglist").find("tr:nth-of-type(2)");
        this.row.remove();
        $("#listfile").click(function() { t.getlist(); });
//        $("#vchart").hide();
        t.chart = new BrewChart("vchart-canvas");
    },
    view: function(name, date) {
        //console.log("view chart:"+name);
        $("#vchart").show();
        $("#log-viewlogname").text(name + "@" + date.toLocaleString());
        var t = this;
        t.chart.clear();

        t.chart.setCelius(this.bm.celius);
        this.bm.getBinary('GET',t.url + "?" + t.dl + "=" + name,
            function(data){
                if (data.length == 0) {
                    //console.log("zero content");
                    return;
                }
                //t.chart.realtime = true;
                t.chart.process(data);
            },function(e) {
            console.log("error");
            });
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
            nr.find(".dlbutton").click(function() {
                window.open(t.bm._url(t.url + "?" + t.dl + "=" + name));
            });
            nr.find(".viewbutton").click(function() {
                t.view(name, date);
            });
            tb.append(nr);
        });
    },
    getlist: function() {
        var t = this;
        t.bm.getFile(t.url,
            function(json) {
                $("#loglist tr:gt(0)").remove();
                t.process(json);
            },
             function(xhr, status, errorThrown) {
                console.log("Error:" + errorThrown);
            }
        );
    }
};