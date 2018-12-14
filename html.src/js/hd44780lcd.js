document.addEventListener("DOMContentLoaded", function() { var i, c = document.getElementById("canvas"),
                r = c.getBoundingClientRect(),
                u = c.getContext("2d"),
                e = new Image;
            e.onload = function() { c.width = this.width, c.height = this.height, u.drawImage(this, 0, 0), i = u.getImageData(0, 0, c.width, c.height) }, e.src = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAhEAAARTAQMAAADvEOMMAAAABlBMVEX///8AAABVwtN+AAARIElEQVR42uzdz2v0RBgH8EkTkoixu15kqqsbEcSDhxRFe5DdHjx48A/wmLLgD/BQ6KWHtU5dXD3UH6CHioLiHyDeBBG7S2FVKCp4URSNFPWiNnhxwa11JpM3T9dZ285MjN06Acuu7/t+mHSSbPLdJ0/QifZCSjHeQLCQ7KfD36xmP13+Jsx+LvA39eynxd9g+t9z2SubvfRVDZIUBlY2GoWxomzYhBm3/PTZA3cpGyjJjPueePdxdaPBjJvvffqtj9UNm2TGZjhRN1BKjXvue+ItX91gK2P99sNnoY7BVmaB/UsNA61rGjuIrQxdNA2b5Ear9twwdax03PLQaqu2PUxdNO56NgnZ6/GClTZaNqnXSLLetVDLswYJRoQafGVQk75r1z76fuxY40mbGu3acDh20WSLGez1ZMEaN9rMGCTrWxZqF0bz0W83b2PG64hsND7t7Dr2awcbTbS60fhof9e1Dn5s1khIX792sGDvdjdqpN7Y/z790UIbTW8/wTY1lt75ImYrg/cR6eCDDnbspZ1OG6128Gf72LV2Ru0mCenrpZ0FG086TVLHHx2mIwt16LAT7FFj+Z29VbYy+M0J6ez+vNFw7Ftf7LxHjd1f9huu9eJH999CjV/2b31xwW5803mdGj8ern9EjfduGCXYpcaN77+7ihbZujx0yniOGd9zw7Op8T03Gp0eNZIkM5679vtYev2VVTYz+OtNVeOlz5ZuW6UzQ42GqpFvGoto6euDU79Tjxp4yH+nmTHkv9NGx6YGndsRNbxpwxo0Xx+fmluPze2Qzy0zGkM+t40NajSoweZ2yuCbmeI2Bsaixra+7e3nK6O+z/W8w0H2ZlPjc86NuLGobjzr3nFE+MooGy94h5/wdyvKxmvuPZ/lM6NsvNm/Z5/kM6NqbLsbX5F8ZZQN7/AI8cVTNEo5lyL651LnG9bvR4QazQ8n1Nj64/cBNWoDY1zYoB862gZ+uK49t/WHhHWZINRGoTdA1piOA6EmoUbzLMMvw0hLMOr6xuP6hv9gpD0vtV4ijqO2x4ypcaAtarSOf09mb6fGAMMex0NyYWP2vCz462CcO7ctRHfgGcbnbwvj8JJ2tu9bY77vn2fgl7UNO31efxzL41e1jaVA37jO2SWq+9wA22hAjUXn+b8bxynf91vZOApD2MbsdKu2n+aeqoE7QQfrGS7ex4+qG9v9tXcP7vOH+E5f3djpLO88Ggx37wuoseATBYN8ed8t7qgwVlZVjqebt9XswnAeF4wJ+sdxFMZ6Y9HuF4Yfl2CEKsYjN12/MOLzomwc3LPUf5RvHwoGzG1n5bHgcAUM2Wsgd5h8uZy2vGGaGatnngeJ4xD2ffq3yjCItmGVYNDlshpnzK0xqjHKuyZEMI6LLcYwhjEuasxJltPNDC9hRrM1YQY91zaGglH+vFi/s3HQBMhK82tTY1RgVLTPieOo7XWZcZzWiDGmjct6PnbRdWHCmJ1re0fk8hr+SqRvTOoKBuQwIX19Yx/LzwvkMIOYvl7u+YLBx/Fd65iNw075NvbUmG1j9PJoKoeJE/o6sgN5A3KYNWaEriNvQA5zB1uXWMWAHCbKjMCVNyCH4QZ2VXMYMNanDXH78AaoTv+PNc6MGoEMpTASfWNt4MgbkMOEzLj9RMGAHCZmxvJJIG9ADpMwI0K+vAE5zIAZdYS1j4UYCfs+yx2KcaA/YBvj4xANH0XaBl2Mcek/K//vhslyjGGM+TDmIMtpW2P4bPiuBTUkXmIMSaP8eZkg1PzbNZCXGKNso/q55UZbGEe7S40WjZuMIRiVnY/VhH3/iCCWO0zoHxmjCsOPIqRel4PIgGdKZ1/f2inix3U2jr/nQVYvhUxJycAdu4fZtXrflzYgD/L6zIjcQNqAPOiGvg+ZkpQBedB2P6BGvOJIzgvkQYWxKOQwVoqE3AHGAXkQGK6sAVlOYdj6xlpP3oA8yOr7zHjGkTYgD7Ky7SMcyRuQB1m9FWpE3UCjLsfaTmdmSiwU3IN9nx/Xj/a6/1xDUkdY28BU0TV8FGkbdLmgYWpq/n+GyXKMYYz5MOYmy6GfL2+erkP5jhhDxSj/er9G4HzMGP+9UcLcNvee+iOB6yhm8HFAHcrviTEEo+J9jhunc9zW1Tf8ICrBqCsYkMOE7HquoVOXY5GsVuEBX9jG8hyXG5DjwjYGOQxJmHF3IG9ADrPNjPBhR96AHKaX1aEcKBiQwxBmrO248gbkMNwIdOpyuNE5cc/I12Ecs2pqyjTiwJE3IIfJjFDBgByGG1EQyBuQw3AD+fIG5DCk5Loc8Xsx1IQclx/X4R48MUPRNuhyNY25zi6MYbIcYxhjHo1/O8vpTn3nmV0D5bkD/8ye8OOHMco0Kspy8uvbvJZ2wgw7RQjGYQzBqCCHIbzGOS3yoGIctT02jtZxaozyjYr2OX4NJIzDS1BWvz4xxgUMH0XKxsCx0YAZf07dq2En/Pts0bAEw04D3uumPsDKBm7wXjcR8VUNFz+AH+VGIGtAlvMk76ly+weOrAFZToDvy2pIfpU1IMspjL5yllMYsT2VKU1lfRMkjGMqhynTWOtJG5DlBPhOqKmRMiDLCfj2EY6kDchyguBQu6Ym8IZQU6PZrw/hs86l4Lie7ftQbynkQZqGjyJtgy6X0Jjn7MIYJssxhjHmMYf5d7Oc5t50zebxP/X/MIa+UXntBrKT7Lu1qVqFJjGGYFT7mX2tbovAOKC3C+/lSoxRvlFdrxuE2lADD99n5+O4vIa/Eukb3bq2UR9h9fujHN4vZ/kZH4w2gnHw7QPuTXgKxgFZTt4vhwTKBm7wfjm3rzjyBmQ5a8xY21IwIMu5I4YMRc6ALCfKjJ6sAVkOGDpZjmg0icUy+okwDqjb8o6mcxhuPONoG+FIwYAsJ4whQ5EzIMuJoaZG0oAsJ4GaGuUsZxALGQobB9xrfpzm9XRCDzMhQ9E26GKMS5c7GMNkOcYwxjwa83B/VN5X8ogIuUO7a4wqjfLrP3g9bipmFzX6R8YQjSrrciZo1jjQtdzBGFUY5R9Pr+W4ROxnUIzjahs+irSN607qGlkO73VTd/Hp7cOG61tx+xCzHN7rJnJ8ZQM3eK+bKAikDchyPN7LBDvSBmQ5vNdNrGBAlmPx/jCRK2lAlgOGcpYDRsgNOK5PjwONvQG6Ng72CDQEOUxhxGUYjrQBWY7FexApGJDlWLwXUhxIG5Dl5L1ukK+R5VjbqZgH2Yn2PVZdZliQ0fM6FFgXKz2jrk/LoMvVNOY6u7hKhuI1oclyjGEMk+WInw1QQ4KacK5tjAqMSmoEJkI/WF6PawzBqGRexO+zZ9YqGKN8o+rex4Pi2T4wDnYbWMkG9PrVNyZ1baPexwoG5DAh9PpV63UTWCTOjHN7/Rb7fjEOyGFIAnUXcgbkMNvMiDcVDMhheK+bT115A3IYwutQFAzIYbjRUzAghyFqdTmQoYBxod8pgm1MNMKRggE5DIGaGjkDchjoUyNrQA4DfWpkDchhuIGRfH56fq9feJ7ncQqZ0l4XMqUZx0JjaBjmHqt5MUyWYwxjzIcxB1kOKnqpCc9/Mca0cfk/Gy6aXXiJMQSj6iwHxlGcj3lJu7gfyBjlG5XNLdTAQw+RI2KN+bn2FTd8P1I2Qpz3usHq15XxVt7rZgWfv33wTEn4Pjvp5L1uNn1Vw0r28143XwbyBs9y7omHvKdKiB05A+ou4ni4y/vDqBg8yxmA4cobPMsRDIUshwjGecd12Mb0DaipGfB5YYYjb/AsJ47z7SOUN2BuH8t73eBA3uBZThS3yu91M/tc6mL5OqaKruGjSNugyxU1Lsv56f/dMDmMMYwxH8YcZDlwri3UkLSNIRrV5zDMaBV1BsaowKggP509jhrcd2IMwags197j44B7E1C7GIcxRANymBB67sgZkMMMYmZMFLIcyGHiRHgWODyznhpFz9BiHNltYFM5zBozlnu+tAE5DO9TE9mBvAE5DO/L4jrSBuQw3IhVDJ7DgBFo5TDcwO7MXq6zx8G3D8FYVzAghwkzI1EwIIeJmbE2cOQNyGESZtx+omBADsP7Op0EMoa475ef5cC5VF5DAj2YhWcLClmOruGjSNugy9U05jr/uEqGyXKMYYz5MOYgy5k22Pem8MylrjEEo8IsBz6zEZpwo0mMUb1Rfr8cOxHGkV/f5s8VPSLGEIxKnkEFRnGeDPu+l5RtwP1R+sakrmdA/qFkhBiRAdwfpWTEW3mfGjfQyHLsHub5x9nb2BjBOISaGt6nJl6RNyDLyfvULLrSBmQ527wviytrQJZTGLa0AVkOM/TujyKF8Ywz65hcPHNpKzO8hI9jMtMIR/IGZDm8T03UDaQNyHLyPjXIlzYgy+F9auoISxuQ5fA+NZgq0oaYf/yL55ZNeJ4nz+jz47oxpgxTU/M/MkyWYwxjzIdRWZYD9ZZ/5M+eRKfOk41RrlFtliOej+U97oyRG3PX829qXVpIONe2UmMIRgXPfePPrOf7/nfo9DjamTG58oYfRAoGZDlhZgg1NRc24q28/0cDa2Q5vNdN9IB/kW3siKCZWc52ZtwdSBuQ5fBeN+HDjrwBWQ5hRnwgb0CWw421HVfOgCwHDK26HG50TsR6GG/Gcb0N4yjHgCwnM+LAkTYgy+FGqGBAlsONKAikDchyoOeOtCHW1OgdT2f33LHSGedSZ2bSKNI26HKVjUtwDmMMk+UYwxj/F6OiLKc24zm8pz+zjVGOUXlmAOdjwr0JdBzGAKPaeYHzMfhuDcZhjBKN6ucWnoUFNSTCOMo2fBTpG3/WtY36ACsbocN73UTEV+91E/BeN9HUs8C7UttH0uC9bm7/wFE1rOQB3stk7VdpA7KcJ3lPlbW+K2vA9X6AeW8XW9aALEfdgCyHGbp1OYXxzKz846xtTDTCkawBWQ417oS6HCkDspwg74WEfGkD5jY4hLocKQOynMAbpuX2LT7f6Er2+K9Br6zp+4Gs1BjXjHn7PsoYJssxxl/t3UFOwlAQgOFHbACTJnbpwsR6A9y5kquUW7gwpkDCuVi59Ar2BrJzIUQDU/mDFeK8ltqS2bH68hKIKb/DPDPaYLTs91F553cf73OMpRnHMmp9b1/daiG/O8HoLMzAqPE5iDvKLub3ne05em9p/jxmRuVG/XM5u+fIhnJnfS9rsMGeGr0RB/mum8fI20hC2XUTPZeYqbmSXTe3k/6+z4crnKPYYUYbIw3VBh1Gdt3c3AV6gw4zWBujJ61Bh9kas67OoMNgTLUGDQVD13JUBudgf3Kxw8QJPUhn0GES7rFSGnSYjHuslAYdZs5MjcKoa6am+BzEORS/01IZ8tqMxnUHM6zlmGFGG43WtBxmNqXzm9EMo9K5HO4IoV2Y8U8th/+t7dwjMfyekzajUqP+2fOho1047lza7DNortF3g9LG+WfkbcSB7MuJupcl5nLyfTlBf++s5IpzZI6m5OhBsi9nEIZqgx7Um3HHkM6gB+X7cjwMmkFHdt0MukqDHuRt0IMwYv+WszWSwnwQf9c5B+3iVyNQG/Qg2ZcTexj0oHxfThLqDd7b6R0tR2XQgzrjxRFaDnNbe85xeLbH35DXZjSuO5hhLccMM9potKflbO9cMqNJRtn39izbvVc0N2gXZhSMuu46cj/PwZz00ozKjPr3GPH9tvisLe3iNI11h4nZwawz6DBp6TuormTXTbSzg9kd+n7LZ4wOM87Ywawz6DDTZGOEeoPv6inzHzqDDiNG8qA06DAYL121QUMRYzTxbjkYU4zrFOMvHcbfoMNgBHqj+N7qDTpMymyPxjj2HVQYPe5LYk/vo2K2R2vQg/wNeX3SRjvbxSkZ1mHMMKMdRgUt5wtX+aON07+erQAAAABJRU5ErkJggg=="; var d = document.getElementById("info"),
                t = d.textContent;
            c.addEventListener("mousemove", function(e) { var t = e.clientX - r.left,
                    n = e.clientY - r.top,
                    a = u.getImageData(t, n, 1, 1).data,
                    o = "rgba(" + a[0] + "," + a[1] + "," + a[2] + "," + a[3] + ")";
                d.textContent = "x=" + t + " y=" + n + " " + o }), c.addEventListener("mouseleave", function(e) { d.textContent = t }); var g = document.getElementById("scan");
            g.addEventListener("click", function() { Date.now(); for (var e = 0; e < 6; e++)
                    for (var t = 0; t < 16; t++) { var n = v(Math.round(214 + 52.3 * e), Math.round(40 + 66.9 * t));
                        h.push(n) }
                u.putImageData(i, 0, 0), Date.now(), document.getElementById("after").style.display = "block", document.getElementById("after").style.visibility = "visible", g.disabled = !0; for (var a = document.getElementsByClassName("hide"), o = 0, c = a.length; o < c; o++) a[o].style.display = "none";
                w() }); var n = document.getElementById("base");
            n.onchange = w; var a = document.getElementById("braces");
            a.onchange = w; var o = document.getElementById("comment");
            o.onchange = w; var s = document.getElementById("output");
            document.getElementById("copy").addEventListener("click", function(e) { s.focus(), s.select(); try { document.execCommand("copy") } catch (e) {} }); var m, l, h = [
                    [0, 0, 0, 0, 0, 0, 0, 0],
                    [4, 4, 4, 4, 0, 0, 4, 0],
                    [10, 10, 10, 0, 0, 0, 0, 0],
                    [10, 10, 31, 10, 31, 10, 10, 0],
                    [4, 15, 20, 14, 5, 30, 4, 0],
                    [24, 25, 2, 4, 8, 19, 3, 0],
                    [12, 18, 20, 8, 21, 18, 13, 0],
                    [12, 4, 8, 0, 0, 0, 0, 0],
                    [2, 4, 8, 8, 8, 4, 2, 0],
                    [8, 4, 2, 2, 2, 4, 8, 0],
                    [0, 4, 21, 14, 21, 4, 0, 0],
                    [0, 4, 4, 31, 4, 4, 0, 0],
                    [0, 0, 0, 0, 12, 4, 8, 0],
                    [0, 0, 0, 31, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 12, 12, 0],
                    [0, 1, 2, 4, 8, 16, 0, 0],
                    [14, 17, 19, 21, 25, 17, 14, 0],
                    [4, 12, 4, 4, 4, 4, 14, 0],
                    [14, 17, 1, 2, 4, 8, 31, 0],
                    [31, 2, 4, 2, 1, 17, 14, 0],
                    [2, 6, 10, 18, 31, 2, 2, 0],
                    [31, 16, 30, 1, 1, 17, 14, 0],
                    [6, 8, 16, 30, 17, 17, 14, 0],
                    [31, 17, 1, 2, 4, 4, 4, 0],
                    [14, 17, 17, 14, 17, 17, 14, 0],
                    [14, 17, 17, 15, 1, 2, 12, 0],
                    [0, 12, 12, 0, 12, 12, 0, 0],
                    [0, 12, 12, 0, 12, 4, 8, 0],
                    [2, 4, 8, 16, 8, 4, 2, 0],
                    [0, 0, 31, 0, 31, 0, 0, 0],
                    [8, 4, 2, 1, 2, 4, 8, 0],
                    [14, 17, 1, 2, 4, 0, 4, 0],
                    [14, 17, 1, 13, 21, 21, 14, 0],
                    [14, 17, 17, 17, 31, 17, 17, 0],
                    [30, 17, 17, 30, 17, 17, 30, 0],
                    [14, 17, 16, 16, 16, 17, 14, 0],
                    [28, 18, 17, 17, 17, 18, 28, 0],
                    [31, 16, 16, 30, 16, 16, 31, 0],
                    [31, 16, 16, 30, 16, 16, 16, 0],
                    [14, 17, 16, 23, 17, 17, 15, 0],
                    [17, 17, 17, 31, 17, 17, 17, 0],
                    [14, 4, 4, 4, 4, 4, 14, 0],
                    [7, 2, 2, 2, 2, 18, 12, 0],
                    [17, 18, 20, 24, 20, 18, 17, 0],
                    [16, 16, 16, 16, 16, 16, 31, 0],
                    [17, 27, 21, 21, 17, 17, 17, 0],
                    [17, 17, 25, 21, 19, 17, 17, 0],
                    [14, 17, 17, 17, 17, 17, 14, 0],
                    [30, 17, 17, 30, 16, 16, 16, 0],
                    [14, 17, 17, 17, 21, 18, 13, 0],
                    [30, 17, 17, 30, 20, 18, 17, 0],
                    [15, 16, 16, 14, 1, 1, 30, 0],
                    [31, 4, 4, 4, 4, 4, 4, 0],
                    [17, 17, 17, 17, 17, 17, 14, 0],
                    [17, 17, 17, 17, 17, 10, 4, 0],
                    [17, 17, 17, 21, 21, 21, 10, 0],
                    [17, 17, 10, 4, 10, 17, 17, 0],
                    [17, 17, 17, 10, 4, 4, 4, 0],
                    [31, 1, 2, 4, 8, 16, 31, 0],
                    [28, 16, 16, 16, 16, 16, 28, 0],
                    [17, 10, 31, 4, 31, 4, 4, 0],
                    [14, 2, 2, 2, 2, 2, 14, 0],
                    [4, 10, 17, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 31, 0],
                    [8, 4, 2, 0, 0, 0, 0, 0],
                    [0, 0, 14, 1, 15, 17, 15, 0],
                    [16, 16, 22, 25, 17, 17, 30, 0],
                    [0, 0, 14, 16, 16, 17, 14, 0],
                    [1, 1, 13, 19, 17, 17, 15, 0],
                    [0, 0, 14, 17, 31, 16, 14, 0],
                    [6, 9, 8, 28, 8, 8, 8, 0],
                    [0, 15, 17, 17, 15, 1, 14, 0],
                    [16, 16, 22, 25, 17, 17, 17, 0],
                    [4, 0, 12, 4, 4, 4, 14, 0],
                    [2, 0, 6, 2, 2, 18, 12, 0],
                    [16, 16, 18, 20, 24, 20, 18, 0],
                    [12, 4, 4, 4, 4, 4, 14, 0],
                    [0, 0, 26, 21, 21, 17, 17, 0],
                    [0, 0, 22, 25, 17, 17, 17, 0],
                    [0, 0, 14, 17, 17, 17, 14, 0],
                    [0, 0, 30, 17, 30, 16, 16, 0],
                    [0, 0, 13, 19, 15, 1, 1, 0],
                    [0, 0, 22, 25, 16, 16, 16, 0],
                    [0, 0, 14, 16, 14, 1, 30, 0],
                    [8, 8, 28, 8, 8, 9, 6, 0],
                    [0, 0, 17, 17, 17, 19, 13, 0],
                    [0, 0, 17, 17, 17, 10, 4, 0],
                    [0, 0, 17, 21, 21, 21, 10, 0],
                    [0, 0, 17, 10, 4, 10, 17, 0],
                    [0, 0, 17, 17, 15, 1, 14, 0],
                    [0, 0, 31, 2, 4, 8, 31, 0],
                    [2, 4, 4, 8, 4, 4, 2, 0],
                    [4, 4, 4, 4, 4, 4, 4, 0],
                    [8, 4, 4, 2, 4, 4, 8, 0],
                    [0, 4, 2, 31, 2, 4, 0, 0],
                    [0, 4, 8, 31, 8, 4, 0, 0]
                ],
                x = 5,
                G = 8,
                M = 27 / (x - 1),
                f = 46 / (G - 1),
                j = [];

            function v(e, t) { j = []; for (var n = 0; n < G; n++) { for (var a = m = 0; a < x; a++) { l = Math.round(e + a * M); var o = 4 * (Math.round(t + n * f) * c.width + l);
                        i.data[o] || (m |= 16 >> a), i.data[o] = 255, i.data[o + 1] = 0, i.data[o + 2] = 0, i.data[o + 3] = 255 }
                    j.push(m) } return j }

            function w() { var c = n.value,
                    i = a.value,
                    r = o.value,
                    u = "";
                u += r + " HD44780 5x7 pixel font data\n", u += r + " http://eleif.net/HD44780.html \n", u += r + " Array offset from ASCII code is decimal 32 (hex 20), i.e. the first element with index 0 is SPACE (' ').\n", u += r + " Bit mapping for 'e':\n", u += r + "     76543210\n", u += r + " 0   xxx     \n", u += r + " 1   xxx     \n", u += r + " 2   xxx 111 \n", u += r + " 3   xxx1   1\n", u += r + " 4   xxx11111\n", u += r + " 5   xxx1    \n", u += r + " 6   xxx 111 \n", u += r + "     [cursor]\n", u += r + "\n", u += r + "\n", "2" == c && (u += r + "            0           1           2           3           4           5           6\n"), "10" == c && (u += r + "     0    1    2    3    4    5    6\n"), "16" == c && (u += r + "      0     1     2     3     4     5     6\n"), u += "fontdata", u += "[" + h.length + "][7]", u += " = " + i[0] + "\n", h.forEach(function(e, t, n) { u += "    " + i[0]; for (var a = 0; a < 7; a++) { var o = e[a]; switch (c) {
                            case "2":
                                u += "0b" + ("00000000" + o.toString(2).toUpperCase()).slice(-8); break;
                            case "10":
                                u += ("   " + o.toString(10).toUpperCase()).slice(-3); break;
                            case "16":
                                u += "0x" + ("00" + o.toString(16).toUpperCase()).slice(-2) }
                        a < e.length - 1 && (u += ", ") }
                    u += i[1], t < h.length - 1 ? u += ", " : u += "  ", u += r + " [" + String(t).padStart(2) + "] '" + String.fromCharCode(32 + t) + "'\n" }), u += "  " + i[1] + ";\n", s.textContent = u } var y = document.getElementById("download"),
                A = "HD44780_font.txt";
            y.value = "Save as '" + A + "'", y.addEventListener("click", function(e) { H("data:," + encodeURIComponent(s.textContent), A) }); var B = document.getElementById("lcd"),
                D = B.getContext("2d");
            D.imageSmoothingEnabled = !1, D.webkitImageSmoothingEnabled = !1, D.msImageSmoothingEnabled = !1, B.height = 35, B.width = 240, B.style.padding = "4px", B.style.height = 2 * B.height + "px", B.style.width = 2 * B.width + "px"; var I = new ImageData(new Uint8ClampedArray([0, 0, 0, 255]), 1, 1);

            function N(e, t, n) { for (var a = 0; a < G; a++)
                    for (var o = x; 0 <= o; o--) h[e.charCodeAt(0) - 32][a] & 1 << o && D.putImageData(I, t + x - 1 - o, n + a) }

            function Y(e, t, n) { for (var a = x + 1, o = 0; o < e.length; o++) N(e[o], t + a * o, n) } var b = document.getElementById("input");

            function E(e) { if (h.length) { D.clearRect(0, 0, B.width, B.height); var t = []; if (e.type) { for (var n = "", a = 0, o = b.value, c = b.value.length; a < c; a++) 40 == n.length || "\n" == o[a] ? "\n" == o[a] ? (t.push(n), n = "") : (t.push(n), n = o[a]) : n += o[a];
                        t.push(n) } else t = e.split("\n"); for (a = 0; a < t.length; a++) Y(t[a], 0, a * (G + 1)) } else alert("No font data yet. Click the button to scan the image first.") }
            b.addEventListener("input", E), E("Hi there,\nhow are you?\nIt's " + (new Date).toLocaleTimeString("en-GB", { hour: "numeric", minute: "numeric" }) + " (not updated)\nJust type into the text box above."); var z = document.getElementById("savelcd"),
                p = 0,
                O = "HD44780_";

            function H(e, t) { var n = document.createElement("a");
                n.href = e, n.setAttribute("download", t); var a = document.createEvent("MouseEvents"); return a.initEvent("click", !1, !0), n.dispatchEvent(a), !1 }
            z.value = "Save as '" + O + ("0" + p).slice(-2) + ".png'", z.addEventListener("click", function(e) { var t = B.toDataURL("image/png"),
                    n = O + ("0" + ++p).slice(-2) + ".png";
                H(t, n), z.value = "Save as '" + n + "'" }); var L = document.getElementById("clock");! function e() { var t = new Date;
                L.innerHTML = ("0" + t.getHours()).slice(-2) + ":" + ("0" + t.getMinutes()).slice(-2) + ":" + ("0" + t.getSeconds()).slice(-2), setTimeout(function() { e() }, 1e3) }(); for (var Z = document.getElementsByClassName("note"), k = 0, U = Z.length; k < U; k++) Z[k].addEventListener("click", function() { this.style.height ? this.style.height = "" : this.style.height = "1.4em" }); /*(adsbygoogle=window.adsbygoogle||[]).push({})})*/