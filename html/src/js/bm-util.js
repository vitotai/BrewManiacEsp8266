// the value is raw data direct from EEPROM
function setCookie(a, b, c) {
    var d = new Date();
    d.setTime(d.getTime() + (c * 24 * 60 * 60 * 1000));
    var e = "expires=" + d.toUTCString();
    document.cookie = a + "=" + b + "; " + e
}

function getCookie(a) {
    var b = a + "=";
    var d = document.cookie.split(';');
    for (var i = 0; i < d.length; i++) { var c = d[i]; while (c.charAt(0) == ' ') { c = c.substring(1) } if (c.indexOf(b) == 0) { return c.substring(b.length, c.length) } }
    return ""
}

var BrewMath = {
    plato2sg: function(plato) {
        return 1 + plato / (258.6 - ((plato / 258.2) * 227.1));
    },
    sgTempCorrected: function(Reading, F, CT) {
        return Reading * ((1.00130346 - 0.000134722124 * F + 0.00000204052596 * F * F - 0.00000000232820948 * F * F * F) / (1.00130346 - 0.000134722124 * CT + 0.00000204052596 * CT * CT - 0.00000000232820948 * CT * CT * CT));
    },
    brix2sg: function(brix, wc) {
        wc = (typeof wc === "undefined") ? 1.0 : wc;
        var bi = brix / wc;
        return (bi / (258.6 - ((bi / 258.2) * 227.1))) + 1;
    },
    sg2plato: function(sg) {
        return (1111.14 * sg) - (630.272 * sg * sg) + (135.997 * sg * sg * sg) - 616.868;
    }
};