// JavaScript source code

Module["createViewBoxPlayerSkin"] = function (pp) {
    function ImageButton(info) {
        this.m_ele = null;
        this.m_ele_width = info.width;
        this.m_ele_height = info.height;
        this.m_switch = false; // false status1, true 2
        this.m_status1 = info.status1;
        this.m_status1hover = info.status1hover;

        if (typeof info.status2 != 'undefined') {
            this.m_status2 = info.status2;
            this.m_status2hover = info.status2hover;
        } else {
            this.m_status2 = null;
        }

        if (typeof info.dontlistenmouse != 'boolean') {
            this.m_dontlistenmouse = false;
        } else {
            this.m_dontlistenmouse = info.dontlistenmouse;
        }
    }
    ImageButton.prototype.init = function (info) {
        this.m_ele = document.createElement('button');
        this.m_ele.setAttribute('style', "position:absolute; border:none;");
        this.m_ele.style.width = new String(this.m_ele_width) + "px";
        this.m_ele.style.height = new String(this.m_ele_height) + "px";
        this.setBg(false);

        this.m_ele.style.top = "50%";
        this.m_ele.style.transform = "translate(-50%, -50%)";
        this.m_ele.style.padding = "0";
        this.m_ele.style.outline = "none";
        this.m_ele.style.cursor = "pointer";
        this.m_ele.style.overflow = 'hidden';
        this.m_ele.m_customcb = this;

        if (!this.m_dontlistenmouse) {
            this.m_ele.addEventListener('mouseenter', onPMouseEnter);
            this.m_ele.addEventListener('mouseleave', onPMouseLeave);
        }
        this.m_ele.addEventListener('click', onPClick);
    }
    ImageButton.prototype.setBg = function (bhover) {

        if (!this.m_switch) {
            if ((bhover ? this.m_status1hover : this.m_status1).indexOf("data:image/") == 0) {
                this.m_ele.style.background = "url(" + (bhover ? this.m_status1hover : this.m_status1) + ")";
            } else if ((bhover ? this.m_status1hover : this.m_status1).indexOf("rgb") == 0) {
                this.m_ele.style.background = (bhover ? this.m_status1hover : this.m_status1);
            }
        } else if(this.m_status2 != null){
            if ((bhover ? this.m_status2hover : this.m_status2).indexOf("data:image/") == 0) {
                this.m_ele.style.background = "url(" + (bhover ? this.m_status2hover : this.m_status2) + ")";
            } else if ((bhover ? this.m_status2hover : this.m_status2).indexOf("rgb") == 0) {
                this.m_ele.style.background = (bhover ? this.m_status2hover : this.m_status2);
            }
        }
    }
    ImageButton.prototype.handleMouseEnter = function (e) {
        this.setBg(true);
    }
    ImageButton.prototype.handleMouseLeave = function (e) {
        this.setBg(false);
    }
    ImageButton.prototype.handleClick = function (e, c) {

        if (e.target.id == 'plorpa') {
            if (!this.m_switch) {
                if (c.startPlay()) {
                    //this.changeStatus(true);
                }
            } else {
                if (c.pausePlay()) {
                    //this.changeStatus(false);
                }
            }
        } else if (e.target.id == 'viewshareplayerflsn') {
            if (!this.m_switch) {
                c.enterFullSceen();
                //this.changeStatus(true);
            } else {
                c.leaveFullScreen();
                //this.changeStatus(false);
            }
        } else if (e.target.id == 'viewshareplayervolume') {
            c.showVolumeBar(true, e);
        } else if (e.target.id == "viewshareplayersubtitle") {
            c.showSubtitleBar(true, e);
        }
    }
    ImageButton.prototype.changeStatus = function (b) {
        this.m_switch = b;
        this.setBg(true);
    }

    function onPMouseEnter(e) {
        this.m_customcb.handleMouseEnter(e);
    }
    function onPMouseLeave(e) {
        this.m_customcb.handleMouseLeave(e);
    }
    function onPMouseDown(e) {
        this.m_customcb.handleMouseDown(e);
    }
    function onPMouseUp(e) {
        this.m_customcb.handleMouseUp(e);
    }
    function onPMouseMove(e) {
        this.m_customcb.handleMouseMove(e);
    }
    function onPClick(e) {
        this.m_customcb.handleClick(e, this.m_container);
        document.activeElement.blur();
    }
    function onPKeyDown(e) {
        if (e.keyCode == 32 || e.code == 'Space') {
            var v = document.getElementById('plorpa');
            var cb, c;
            if (!v) {
                return;
            }

            cb = v.m_customcb;
            c = v.m_container;

            if (cb && c) {
                cb.handleClick({
                    target: {
                        id: 'plorpa'
                    }
                }, c);
            }

            if (!c.uishow) {
                c.setSkinVisible(true);
            }

            e.stopPropagation();
            if (e.preventDefault) {
                e.preventDefault();
            } else {
                window.event.returnValue = false;
            }
        }
    }

    function PlayProgressShow(info) {
        this.tr = null;
        this.m_ele = null;
        this.m_ele_bottom = info.bottom;
        this.m_playedregion = null;
        this.m_buffedregion = null;
        this.m_played = 0;
        this.m_buffed = 0;
        this.m_cursor = null;
        this.m_bcursorclick = false;
        this.m_cursorclickinfo = { px: -0.01 };
        this.m_duration = 0;
    }
    PlayProgressShow.prototype.init = function () {
        this.m_ele = document.createElement('div');
        this.m_ele.id = 'porgerssbar';
        this.m_ele.m_customcb = this;
        this.m_ele.setAttribute('style', "position:absolute; background-color:rgba(0, 0, 0,0.4); width:100%; height:3px; border-radius:3px; ");
        this.m_ele.style.bottom = new String(this.m_ele_bottom) + "px";
        this.m_ele.style.cursor = "pointer";

        this.m_playedregion = document.createElement('span');
        this.m_playedregion.setAttribute('style', "position:absolute; background-color:#e69000; left:0px; height:100%; border-radius:3px;");
        this.m_ele.appendChild(this.m_playedregion);

        this.m_buffedregion = document.createElement('span');
        this.m_buffedregion.setAttribute('style', "position:absolute; background-color:#cccccc;  height:100%;");
        this.m_ele.appendChild(this.m_buffedregion);

        this.m_cursor = new ImageButton({
            width: 12,
            height: 12,
            status1: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAAtklEQVQokY2SPQrCUBCExwgR1CKdRq+jN7FNPFDiNbxCCsEfsE+nsVZBEfws3BdCwJgPphlmH4/ZFSCTD8RABlxNmXm+y7nwFNjxm51l5F5uCjv2QE9A1CLsiGT/LHnka4okpEhCHvm6PpB1gKckX8Ylneh9P0uSvEGo0eKkCk9PTUDdeXmStlUnmKXy+mN1h1MF81V94Ci+PbclFtCzyv5xwGpts7g9lcVVT2MJbICbaWNeeRofrbidwT99kZIAAAAASUVORK5CYII=",
            dontlistenmouse: true
        });
        this.m_cursor.init();
        this.m_cursor.m_ele.id = 'cusrorbuton';
        this.m_cursor.m_ele.m_customcb = this;
        this.m_cursor.m_ele.style.visibility = "hidden";
        //this.m_cursor.m_ele.style.borderRadius = "50%";
        this.m_ele.appendChild(this.m_cursor.m_ele);

        this.setProgressRegion(0, 0);

        this.m_ele.addEventListener('mousemove', onPMouseMove);
        this.m_ele.addEventListener('mouseup', onPMouseUp);
        this.m_ele.addEventListener('mousedown', onPMouseDown);
        this.m_ele.addEventListener('mouseenter', onPMouseEnter);
        this.m_ele.addEventListener('mouseleave', onPMouseLeave);
        this.m_cursor.m_ele.addEventListener('mousedown', onPMouseDown);
        this.m_cursor.m_ele.addEventListener('mouseup', onPMouseUp);
        // mousedown -> mouseup -> click

        this.floatshowtime = document.createElement('label');
        this.floatshowtime.setAttribute('style', 'position:absolute; width:50px; height:20px; line-height:20px; left:0; top:-30px; color:#ffffff; background-color:rgb(80,80,80); font-size:12px; text-align:center; transform:translateX(-50%);');
        this.floatshowtime.innerHTML = '';
        this.floatshowtime.style.visibility = "hidden";
        this.m_ele.appendChild(this.floatshowtime);
    }
    PlayProgressShow.prototype.setProgressRegion = function (played, buffed) {

        if (typeof window.vs_haltplaytime == 'number') {
            if (played >= window.vs_haltplaytime) {
                window.vs_haltplay = true;
                return;
            } else {
                window.vs_haltplay = false;
            }
        }
        if (played < 0 || played > 100) {
            return;
        }
        if (played + buffed > 100) {
            buffed = 100 - played;
        }

        this.m_playedregion.style.width = new String(played) + "%";
        this.m_buffedregion.style.left = new String(played) + "%";
        this.m_buffedregion.style.width = new String(buffed) + "%";
        this.m_cursor.m_ele.style.left = new String(played) + "%";
        this.m_played = played;
        this.m_buffed = buffed;
    }
    PlayProgressShow.prototype.setVisible = function (b) {

        if (b) {
            this.m_ele.style.visibility = 'visible';
            //this.m_cursor.m_ele.style.visibility = 'visible';
        } else {
            this.m_ele.style.visibility = 'hidden';
            this.m_cursor.m_ele.style.visibility = 'hidden';
        }
    }
    PlayProgressShow.prototype.updatePlayTime = function (playtime) {
        if (this.m_bcursorclick) {
            return;
        }

        var pd = playtime / this.m_duration * 100;
        var d = (playtime - this.m_currentTime) / this.m_duration * 100;
        var bd = this.m_buffed - d;
        if (bd < 0 || bd > this.m_buffed) {
            bd = 0;
        }
        this.setProgressRegion(pd, bd);
        this.m_currentTime = playtime;
        //console.log(playtime);
    }
    PlayProgressShow.prototype.updateBuffedTime = function (playtime, maxBuffedTime) {
        if (this.m_bcursorclick) {
            return;
        }

        var bd = (maxBuffedTime - playtime) / this.m_duration * 100;
        this.setProgressRegion(this.m_played, bd);
    }
    PlayProgressShow.prototype.handleMouseDown = function (e) {
        if (e.currentTarget.id == 'cusrorbuton') {
            //console.log('cursor mousedown');
            this.m_bcursorclick = true;
            this.floatshowtime.style.visibility = "hidden";
        } else {
            //console.log('progressbar mousedown');
        }
        this.m_cursorclickinfo = {
            px: e.pageX,
            py: e.pageY
        };
        e.stopPropagation();
    }
    PlayProgressShow.prototype.handleMouseUp = function (e) {

        if (e.currentTarget.id == 'cusrorbuton') {
            //console.log('cursor mouseup');
            if (this.m_bcursorclick) {
                this.m_bcursorclick = false;
                this.m_container.seekTime(this.m_played);
            }
            e.stopPropagation();
            return;
        }

        if (this.m_bcursorclick) {
            this.m_container.seekTime(this.m_played);
            this.m_bcursorclick = false;
            e.stopPropagation();
            return;
        }

        if (e.currentTarget.id == 'porgerssbar') {
            //console.log('progressbar mouseup');
            if (!this.m_bcursorclick && Math.abs(this.m_cursorclickinfo.px - e.pageX) <= 2) {
                var px;
                var played;
                if (typeof e.offsetX != 'undefined') {
                    px = e.offsetX;
                } else if (typeof e.layerX != 'undefined') {
                    px = e.layerX;
                } else {
                    console.error('wtf');
                    return;
                }

                if (e.currentTarget != e.target) {
                    // event offsetX or layerX is for target
                    px += e.target.offsetLeft;
                }

                played = px / this.m_ele.offsetWidth * 100;
                this.setProgressRegion(played, 0);
                this.m_container.seekTime(played);
            } else {
                console.error('is seeking????');
            }
        } else {
            //console.log('uilayer mouseup');
            if (e.target.id == 'uilayer') {
                var v = document.getElementById('plorpa');
                if (v) {
                    if (v.m_customcb && v.m_container) {
                        v.m_customcb.handleClick({
                            target: {
                                id: 'plorpa'
                            }
                        }, v.m_container);
                    }
                }
            }
        }
    }
    PlayProgressShow.prototype.handleMouseMove = function (e) {
        //console.log('progressbar mousemove');
        if (this.m_bcursorclick) {
            var played = this.m_played + (e.pageX - this.m_cursorclickinfo.px) / this.m_ele.offsetWidth * 100;
            var buffered = 0;
            this.setProgressRegion(played, buffered);
            this.m_cursorclickinfo.px = e.pageX;
        } else {
            var pos;
            if (e.offsetX)
                pos = (e.target.offsetLeft + e.offsetX) / this.m_ele.offsetWidth;
            else
                pos = (e.target.offsetLeft + e.layerX) / this.m_ele.offsetWidth;

            this.floatshowtime.style.left = new String(pos * 100) + "%";
            this.floatshowtime.innerHTML = formatTime(pos * this.m_duration);
        }

        e.stopPropagation();
    }
    PlayProgressShow.prototype.handleMouseEnter = function (e) {
        this.m_ele.style.height = "6px";
        this.m_cursor.m_ele.style.visibility = "visible";
        this.floatshowtime.style.visibility = "visible";
    }
    PlayProgressShow.prototype.handleMouseLeave = function (e) {
        this.m_ele.style.height = "3px";
        this.m_cursor.m_ele.style.visibility = "hidden";
        this.floatshowtime.style.visibility = "hidden";
    }

    function PlayVolumeShow(info) {
        this.bottom = info.bottom;
        this.right = info.right;
        this.visible = false;
        this.mouseposinpagex_whenclickvolume = 0;
        this.mouseposinpagey_whenclickvolume = 0;
        this.volume = 1.0;
        this.clickcursor = false;
        this.clickpos = { py: 0 };
    }
    PlayVolumeShow.prototype.init = function () {
        this.m_ele = document.createElement('div');
        this.m_ele.setAttribute('style', ' width:40px; height:163px; position:absolute; background-color:rgba(0,0,0,0.7);');
        this.m_ele.style.right = this.right.toString() + "px";
        this.m_ele.style.bottom = this.bottom.toString() + "px";
        this.m_ele.id = "viewshareplayervolumebar";
        this.m_ele.m_customcb = this;
        this.m_ele.addEventListener('mouseleave', onPMouseLeave);
        this.m_ele.addEventListener('mouseup', onPMouseUp);
        this.m_ele.addEventListener('mousedown', onPMouseDown);
        this.m_ele.addEventListener('mousemove', onPMouseMove);

        this.setVisible(false);

        // slideway
        var slideway = document.createElement('div');
        slideway.setAttribute('style', "position:absolute; background-color:rgba(255,255,255,0.4); width:4px; height:140px; top:12px; left:18px;");
        this.m_ele.appendChild(slideway);

        // indicate volume
        var v = document.createElement('div');
        v.setAttribute('style', "position:absolute; background-color:#e69000; width:100%; bottom:0; height: 50%;");
        slideway.appendChild(v);

        // cursor
        v = new ImageButton({
            width: 12,
            height: 12,
            status1: 'rgb(255, 255, 255)',
            dontlistenmouse: true
        });
        v.init();
        v.m_ele.style.removeProperty('top');
        v.m_ele.style.borderRadius = '50%';
        v.m_ele.style.left = '50%';
        v.m_ele.style.transform = 'translate(-50%, +50%)';
        v.m_ele.style.bottom = '50%';
        slideway.appendChild(v.m_ele);

        this.setCursorPop(1);
    }
    PlayVolumeShow.prototype.handleMouseMove = function (e) {
        var d = 0;
        if (this.clickcursor) {
            d = e.pageY - this.clickpos.py;
            this.setCursorPop(this.volume - d / 140);
            this.clickpos.py = e.pageY;
        }
    }
    PlayVolumeShow.prototype.handleMouseDown = function (e) {
        if (e.target == this.m_ele.firstChild.lastChild) {
            // cursor
            this.clickcursor = true;
            this.clickpos.py = e.pageY;
        } else {
            this.clickpos.px = e.PageX;
            this.clickpos.py = e.pageY;
        }

        e.stopPropagation();
    }
    PlayVolumeShow.prototype.handleMouseUp = function (e) {
        var y, h, v;
        if (this.clickcursor) {
        } else {
            if (Math.abs(e.pageY - this.clickpos.py) <= 2) {

                if (e.offsetX) {
                    y = e.offsetY;
                    h = e.target.offsetHeight;
                } else if (e.layerX) {
                    y = e.layerY;
                    h = e.target.offsetHeight;
                }

                if (e.target == this.m_ele.firstChild.firstChild) {
                    // volume%
                    v = (h - y) / this.m_ele.firstChild.offsetHeight;
                    this.setCursorPop(v);
                } else if (e.target == this.m_ele.firstChild) {
                    // slideway
                    v = (h - y) / h;
                    this.setCursorPop(v);
                }
            }
        }

        this.clickcursor = false;
        e.stopPropagation();
    }
    PlayVolumeShow.prototype.handleMouseLeave = function (e) {
        this.setVisible(false);
        this.clickcursor = false;
    }
    PlayVolumeShow.prototype.setVisible = function (b) {
        this.visible = b;
        this.m_ele.style.visibility = b ? "visible" : "hidden";
    }
    PlayVolumeShow.prototype.recordMouseInpage = function (x, y) {
        this.mouseposinpagex_whenclickvolume = x;
        this.mouseposinpagey_whenclickvolume = y;
    }
    PlayVolumeShow.prototype.setCursorPop = function (r) {
        if (isNaN(r)) {
            console.error("NaN volume");
            return;
        }
        if (r < 0) {
            r = 0;
        } else if (r > 1) {
            r = 1;
        }

        var s = new String(100 * r) + '%';
        // volume %
        this.m_ele.firstChild.firstChild.style.height = s;
        // cursor pos
        this.m_ele.firstChild.lastChild.style.bottom = s;
        this.volume = r;
        //console.log('volume is ' + this.volume);
        if (this.m_container)
        this.m_container.setVolume(r);
    }

    function SubtitleParse(info) {
        this.url = info.url;
        this.xhr = new XMLHttpRequest();
        this.working = false;
        this.xhr.addEventListener("readystatechange", OnXHRStateChanged);
        this.xhr.m_customcb = this;
        this.lines = null;
        this.linesShowNow = 0;
        this.rawLines = null;
        this.rawLineStartRow = 0;
        this.playtime = 0;
        function OnXHRStateChanged(e) {
            var xhr = e.target;
            if (xhr.readyState == 4) {
                if (xhr.status == 200) {
                    xhr.m_customcb.splitSrtLine(xhr.response);
                } else {
                    xhr.m_customcb.splitSrtLine(null);
                }
            }
        }
    }
    SubtitleParse.prototype.request = function () {
        this.xhr.open("GET", this.url);
        this.xhr.responseType = "text";
        this.xhr.send();
        this.working = true;
    }
    SubtitleParse.prototype.splitSrtLine = function (r) {
        if (r != null) {
            this.rawLines = r.split("\r\n");
            //console.error("line " + this.rawLines.length);
            //console.log(this.rawLines);
            this.rawLineStartRow = 0;
            this.linesShowNow = 0;
            this.lines = [{ st: -1, et: 0, line: "" }];
            if (parseInt(this.rawLines[0]) != 1) {
                console.error("error srt file");
                throw "error srt file";
            }
            this.readLineObject();
        } else {
            console.error("failed to open srt file");
            this.working = false;
        }
    }
    SubtitleParse.prototype.readLineObject = function () {
        var i, j, len = this.rawLines.length;
        var no, st, et, t, ms, temp, temp1;
        for (i = this.rawLineStartRow + 1; i < len; ++i) {
            if (this.rawLines[i].length < 1) {

                no = parseInt(this.rawLines[this.rawLineStartRow]);
                temp = this.rawLines[this.rawLineStartRow + 1].split("-->");

                temp1 = temp[0].split(',');
                ms = parseInt(temp1[1]);
                t = temp1[0].split(':');
                st = ms + (parseInt(t[0]) * 3600 + parseInt(t[1]) * 60 + parseInt(t[2])) * 1000;

                temp1 = temp[1].split(',');
                ms = parseInt(temp1[1]);
                t = temp1[0].split(':');
                et = ms + (parseInt(t[0]) * 3600 + parseInt(t[1]) * 60 + parseInt(t[2])) * 1000;

                temp = {
                    st: st,
                    et: et,
                    no: no
                };
                et = 0;
                for (j = this.rawLineStartRow + 2; j < i; ++j) {
                    if (this.rawLines[j].indexOf('\\N') == -1) {
                        temp["line" + et++] = this.rawLines[j];
                    } else {
                        temp1 = this.rawLines[j].split('\\N');
                        temp["line" + et++] = temp1[0];
                        temp["line" + et++] = temp1[1];
                    }
                }
                this.lines.push(temp);

                this.rawLineStartRow = i + 1;
                return true;
            }
        }

        return false;
    }
    SubtitleParse.prototype.getLineByTime = function (t) {
        var i;

        if (t < this.lines[this.linesShowNow].st) {
            i = Math.ceil(this.linesShowNow / 2);
            if (i == this.linesShowNow) {
                return null;
            }
            this.linesShowNow = i;
            return this.getLineByTime(t);
        }

        for (i = this.linesShowNow; i < this.lines.length; ++i) {
            if (t < this.lines[i].et) {
                if (t >= this.lines[i].st - 100) {
                    this.linesShowNow = i;
                    return this.lines[i];
                } else {
                    return null;
                }
            }
        }

        if (!this.readLineObject()) {
            return null;
        }
        this.linesShowNow = this.lines.length - 1;
        return this.getLineByTime(t);
    }

    function PlaySubtitleShow(info) {
        this.subtitle = JSON.parse(JSON.stringify(info.subtitle));
        this.subtitle_playing = {
            id: '',
            no: 0,
            ele: null,
            parser: null
        };
        this.bottom = info.bottom;
        this.right = info.right;
        this.visible = false;
        this.mouseposinpagex_whenclickvolume = 0;
        this.mouseposinpagey_whenclickvolume = 0;
        this.parent = info.parent;
    }
    PlaySubtitleShow.prototype.init = function () {
        var i;
        // region that show subtitle
        this.m_subregion = document.createElement('div');
        this.m_subregion.setAttribute('style', ' position:absolute; height: 60px; left:0; right:0;');
        this.m_subregion.style.bottom = this.bottom.toString() + "px";
        this.parent.appendChild(this.m_subregion);

        i = document.createElement('label');
        i.setAttribute('style', 'position:absolute; text-shadow:2px 0 2px rgb(125,82,0); left:-500px; top:0; right:-500px; height: 40px; line-height:40px; font-family:"Roman"; font-size:25px; color: #ffffff; text-align: center;');
        //i.innerHTML = "中文字幕";
        this.m_subregion.appendChild(i);
        i = document.createElement('label');
        i.setAttribute('style', 'position:absolute; left:-500px; top:40px; right:-500px; height: 20px; font-family:"Roman"; font-size:18px; color: #eeeeee; text-align: center;');
        //i.innerHTML = "english字幕";
        this.m_subregion.appendChild(i);

        // menu for subtitle select
        this.m_ele = document.createElement('div');
        this.m_ele.setAttribute('style', ' width:110px; height:10px; position:absolute; background-color:rgba(0,0,0,0.7);');
        this.m_ele.style.height = new String((this.subtitle.subs_number + 1) * 40 + this.subtitle.subs_number - 1) + "px";
        this.m_ele.style.right = this.right.toString() + "px";
        this.m_ele.style.bottom = this.bottom.toString() + "px";
        this.m_ele.m_customcb = this;
        this.m_ele.addEventListener('mouseleave', onPMouseLeave);

        function leaveLabelItem(e) {
            this.style.color="#ffffff"
        }
        function enterLabelItem(e) {
            this.style.color = "#a1c205";
        }
        function clickLabelItem(e) {
            this.m_customcb.handleSubtitleSelect(this);
        }
        function createItem(info) {
            var liitem = document.createElement('label');
            liitem.setAttribute('style', 'padding: 0;cursor: pointer; border-bottom: 1px solid #888888;  width: 100%;height: 40px; line-height:40px; position:absolute; left:0; font-family:"SegoeUI"; font-size:14px; color: #ffffff;text-align: center;');
            liitem.style.top = new String(info.top) + "px";
            liitem.innerHTML = info.text;
            liitem.m_customcb = info.cb;
            liitem.m_id = info.id;
            liitem.addEventListener('mouseenter', enterLabelItem);
            liitem.addEventListener('mouseleave', leaveLabelItem);
            liitem.addEventListener('click', clickLabelItem);

            return liitem;
        }

        i = -1;
        //  无字幕
        this.m_ele.appendChild(createItem({
            cb: this,
            id: 'none',
            text: "无字幕",
            top: ++i * 40
        }));
        this.subtitle['none'] = null;

        if (this.subtitle['zh'] != null) {
            // 中文字幕
            this.m_ele.appendChild(createItem({
                cb: this,
                id: 'zh',
                text: "中文字幕",
                top: ++i * 40
            }));

            this.subtitle['zh'] = new SubtitleParse({ url: this.subtitle['zh'] });
        }

        if (this.subtitle['en'] != null) {
            // 英文字幕
            this.m_ele.appendChild(createItem({
                cb: this,
                id: 'en',
                text: "英文字幕",
                top: ++i * 40
            }));
            this.subtitle['en'] = new SubtitleParse({ url: this.subtitle['en'] });
        }

        if (this.subtitle['en&zh-cn'] != null) {
            // 中英双语
            this.m_ele.appendChild(createItem({
                cb: this,
                id: 'en&zh-cn',
                text: "中英双语",
                top: ++i * 40
            }));
            this.subtitle['en&zh-cn'] = new SubtitleParse({ url: this.subtitle['en&zh-cn'] });
        }

        this.handleSubtitleSelect(this.m_ele.lastChild);
    }
    PlaySubtitleShow.prototype.handleSubtitleSelect = function (item) {
        this.setVisible(false);

        if (item.m_id == this.subtitle_playing.id) {
            return;
        }

        if (this.subtitle_playing.ele) {
            this.subtitle_playing.ele.style.backgroundColor = "rgba(0,0,0,0)";
        }

        if (item.m_id == 'none') {
            this.m_subregion.firstChild.innerHTML = '';
            this.m_subregion.lastChild.innerHTML = '';
        }

        this.subtitle_playing.ele = item;
        this.subtitle_playing.no = 0;
        this.subtitle_playing.id = item.m_id;

        if (this.subtitle['en&zh-cn'] != null && item.m_id != 'none') {
            this.subtitle_playing.parser = this.subtitle['en&zh-cn'];
        } else {
            this.subtitle_playing.parser = this.subtitle[item.m_id];
        }
        if (this.subtitle_playing.parser) {
            if (!this.subtitle_playing.parser.working) {
                this.subtitle_playing.parser.request();
            }

            function updateUseLineOne(e, l) {
                e.firstChild.innerHTML = l.line0;
            }
            function updateUseLineTwo(e, l) {
                e.firstChild.innerHTML = l.line1;
            }
            function updateEnZh(e, l) {
                e.firstChild.innerHTML = l.line1;
                e.lastChild.innerHTML = l.line0;
            }

            if (this.subtitle_playing.id == 'en&zh-cn') {
                this.subtitle_playing.updateFunc = updateEnZh;
            }else if (this.subtitle_playing.id == 'zh'){
                this.subtitle_playing.updateFunc = this.subtitle['en&zh-cn'] != null ? updateUseLineTwo : updateUseLineOne;
            } else if (this.subtitle_playing.id == 'en') {
                this.subtitle_playing.updateFunc = updateUseLineOne; // english subtitle alway on line one
            }
            this.m_subregion.lastChild.innerHTML = '';
        }

        this.subtitle_playing.ele.style.backgroundColor = "rgb(0,122,204)";
    }
    PlaySubtitleShow.prototype.handleMouseLeave = function (e) {
        this.setVisible(false);
    }
    PlaySubtitleShow.prototype.setVisible = function (b) {
        this.visible = b;
        this.m_ele.style.visibility = b ? "visible" : "hidden";
    }
    PlaySubtitleShow.prototype.updateSubtitle = function (t) {
        if (null == this.subtitle_playing.parser) {
            return;
        }

        t *= 1000;
        var line = this.subtitle_playing.parser.getLineByTime(t);
        if (line == null) {
            this.m_subregion.firstChild.innerHTML = '';
            this.m_subregion.lastChild.innerHTML = '';
            return;
        }

        if (line.no == this.subtitle_playing.no) {
            return;
        }

        this.subtitle_playing.no = line.no;
        this.subtitle_playing.updateFunc(this.m_subregion, line);
    }

    function PlayerSkin() {
        this.ele = pp.wrap;
        this.uri = pp.videouri;
        this.subtitle = pp.subtitle;
        this.uilayer = null;
        this.uishow = true;
        this.progressbar = null;
        this.controlbar = null;
        this.showtime = null;
        this.volumebar = null;
        this.volumebutton = null;
        this.subtitlebar = null;
        this.tid = 0;
        this.tid1 = 0;
        this.stoppos = { x: 0, y: 0, t: 0 };
        this.vinit = false;
        this.autoplay = pp.autoplay;
        this.nofullscreen = pp.nofullscreen;
        this.limittime = pp.limittime;
    }
    PlayerSkin.prototype.init = function () {
        console.log("skin init");
        
        var pe = this.ele.parentNode;
        if (!pe || pe.tagName.toLowerCase() != 'div') {
            console.error("cannt init skin");
            return
        }
        this.ele.removeAttribute('controls');

        this.uilayer = document.createElement('div');
        this.uilayer.m_customcb = this;
        this.uilayer.id = "uilayer";
        this.uilayer.setAttribute('style', "background-color:rgba(0, 1, 2, 0); width:100%; height:100%; position:absolute; top:0; left:0; -webkit-user-select: none;-moz-user-select: none;");
        pe.appendChild(this.uilayer);
        this.uilayer.addEventListener('mouseup', onPMouseUp);
        this.uilayer.addEventListener('mousemove', onPMouseMove);
        this.uilayer.addEventListener('mouseenter', onPMouseEnter);
        this.uilayer.addEventListener('mouseleave', onPMouseLeave);

        //this.videoposter = document.createElement('img');
        //this.videoposter.setAttribute('style', "width:100%; height:100%; position:absolute; top:0; left:0;background-color:rgba(0, 1, 2, 0);");
        //this.videoposter.src = this.videoposterurl;
        //this.uilayer.appendChild(this.videoposter);

        this.controlbar = document.createElement('div');
        this.controlbar.setAttribute('style', "background-color:rgba(0, 0, 0, 0.7); width:100%; height:40px; position:absolute; bottom:0;");
        this.uilayer.appendChild(this.controlbar);

        this.progressbar = new PlayProgressShow({ bottom:40 });
        this.progressbar.init();
        this.progressbar.m_container = this;
        this.uilayer.appendChild(this.progressbar.m_ele);

        this.setSkinVisible(true);

        var v = new ImageButton({
            width: 30,
            height: 30,
            // play normal
            // play hover
            // pause normal
            // pause hover
            status1: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB4AAAAeCAYAAAA7MK6iAAADQElEQVRIiZ3XXWgdRRjG8V+22hJUciz9IDbgB+qJUBWLCuqNXph4oYIR1PZC8EobtUREilUoiAUrgoGaqFcKoq1e1AtveqKgYMHS1k8KTS2ISqHYWpOgYIlUvZhZsjvZc/akfzhw5t2Z59nZfd+Z2Z539jV1wYUYxt24BVehEa/N4iccwqdo4Z86wZ4a4wbGMIrV3dwhfscExuNNVZJ1EHgE09i+BFNYFcdMR42ujZdhEruxdgmGKWujxmTU7Gi8DB9hc4XQTBS5D5djBS7BdTE2GfukbI6aJfP0He/CU8nAs3gdr+rwziINPIdn0JtcmyhqF2f8cIXpz0IWb+vCVOzzAm6NY4s8GT2wMOMGjmFNoeMJ3DY8fexEF4aLaA02B/AVBgrhU2hiNp/xWGJ6FiPR/LyINzyCvwvhNdFLhuVCnRYZFxaEIqvwPvYLCdOpFHMORa0io1ieYUi5Tv/AKxUiO7AJdwgZfBh3dmG+M2rmrMZQblxkN+YqBNYl7ZvwOfYK5dWOuahZZDjDhiTYaiOwE39WxB/AUbyMi9uM3Ze0N2S4Ngl+12bwl0JGvof/kmu9Qhn9iHsrxn6ftK/J0JcEf2tjDCfxKG7HgYrr/dgjrGidNPuqMnNFB+OcA3hMdbldhJV1ApnFidRfM6aB14THN1Bx/TB+SWLpZjOX4XgSbLdB9+Dx2P9Zof6L/Iu3La4SuDFpH8/wdRIcbmO8HW8JC0nKF7gZT6jeoe5J2t9kmEqCGy1OOMLWl/IrHsRd+LbNDfdFzSJTufGZQnAltlYIfFb4/xdexKCwgHRiq3KynUYrwzzeSDqP4foktk3I5OeF2t+hvAFUsT5qFZnEfF5O48KWldMrzOSyQuwc3hXW8ZM1hlqDzXX4WPlAcDp6uSAGZrFFKP6cq7G/Ndi8H0fqjBLW4xNckcS3RK/S1vahcDwpciUOCo+1oZ5G7HuwwnRCYWLpmSs/7I1UiM7gA2HB/8HC4+7HDULJbMKlFWP34iHhdVUa5+a7VJ80z4c38XTRlOpTxDnhlLBROeGWyqmoMZqatjPO2SMsny8p13kdZ+KYpnKylqj7dsrJP9qGLHy05e9yxsJH21T8zdcJ/g8xQbY91wpHzwAAAABJRU5ErkJggg==',
            status1hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB4AAAAeCAYAAAA7MK6iAAADJklEQVRIia3XS2hdRRzH8U9OsaX4yFX6oFqIihqFarDoQl2oC5MsVDCCNgVBV9qgEBEpPqAgKlYECzVR92ofi27cJFFQUGxpfaUIVitqHyi21iQoWCJVFzPHTuaee08S/MKFzH9mfr85mfn/z5yO3Xu7zINz0Ic7cCMuRyP2TeN7HMB7GMdfdYIdNcYNDGMIK+ezQvyKEWyLi6qkaCOwAYewZQGmsCLOORQ15m28BKPYgdULMMxZHTVGo2Zb4yXYjU0VQlNR5C50YRnOxzUxNhrH5GyKmnPM8z3ejkeziafxKl7WZs8iDTyJx7E86xtJtVPj+7EzG/yj8DRf1RjmrMO7uDSLb8Cu1LiBb7AqGXQcN/VPHjm+QFMw1tO1FnuxNgmfQDemyz0ezkxPYyCaL4q44AH8mYRXRS8Flgp5mrJNKAgpK/A2PhYOTLtULDkQtVKGsLRAr7l5+hteqhB5ERtxi3CCP8Vt8zDfGjVLVqK3NE7ZgZkKgYuz9vX4AHuE9GrFTNRM6SuwPguOtxDYit8r4vfgazyP81rMHcva6wtclQW/bDH5I+FEvoV/sr7leAbf4s6KuZNZ+8oCnVnwlxbG8DMewM3YV9G/RqgFF9RodladzGVtjEv24UH8VNF3ruaHaaLQfJDW1Mxp4BUc1Hzg4BMcy2L5y2amwOEs2N3CsAMPx/FPCPmf8jfeFEpsTk/WPlzgsyzY18J4C94QCknOh7gBj5ibsyX9WfvzAhNZcFD1HlU9yVHci9vxRYsFd0bNlInS+FQSvAibKwTeT/7+A8/iaqGAtGNz1Cw5ifECs3gtGzyMa7PY03gITwm5/4K5L4Aq1kWtlFHMtnstfodb+yePVKVMLWM9XZcIe39FEj4ZFz1ddxH4AXf7/y4Cg6VHWkB2CdeTlMuwX/i3NtTTiGP3V5iOSB4sv3OVl72BCtEpvCMU/INC+SQUnOuElNmICyvm7sF9ONPKuDTfrvqmuRhex2OpKdW3iDPCLWFQuCMtlhNRYyg3bWVcslMon8+Zm+d1nIpzujUf1v+o+3YqKT/aep39aCv3csrZj7aJ+JutE/wXUoa3wY3w8wMAAAAASUVORK5CYII=',
            status2: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB4AAAAeCAYAAAA7MK6iAAAAeElEQVRIiWNgGCDAiEti/g51TwYGhrlQbjIDA8N2Is1E0ZfocROrPiY8BsxlYGCQhOK5eNSRpQ+fxZI42IQAUfrwWUxTMGrxqMWjFo9aPGrxqMWjFo9aPGrxqMX0tfg5EvsFCWYSpQ+fxSlQjU8ZIF0YYgG5+ugDAI3SEzonHR3SAAAAAElFTkSuQmCC',
            status2hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAB4AAAAeCAYAAAA7MK6iAAAAeElEQVRIiWNgGCDAiEti1XF5TwYGhrlQbjIDA8N2Is1E0Rdm+RCrPiY8BsxlYGCQhOK5eNSRpQ+fxZI42IQAUfrwWUxTMGrxqMWjFo9aPGrxqMWjFo9aPGrxqMX0tfg5EvsFCWYSpQ+fxSlQjU8ZIF0YYgG5+ugDAJLiEzot+DVGAAAAAElFTkSuQmCC'
        });
        v.init();
        v.m_ele.id = "plorpa";
        v.m_ele.m_container = this;
        v.m_ele.style.left = "35px";
        this.controlbar.appendChild(v.m_ele);

        this.showtime = document.createElement('span');
        this.showtime.setAttribute('style', 'text-align: left; font-family:"黑体"; font-size:14px; color: #ffffff; position: absolute; top: 50%; transform: translateY(-50%); left: 70px;');
        this.controlbar.appendChild(this.showtime);

        v = document.createElement('span');
        v.innerHTML = '00:00';
        this.showtime.appendChild(v);
        v = document.createElement('span');
        v.innerHTML = ' / 00:00';
        this.showtime.appendChild(v);

        // fullscreen
        v = new ImageButton({
            width: 20,
            height: 20,
            status1: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAAzklEQVQ4jc3TPQrCQBCG4XfVyjLYCiK2dp5AwYvYeJRcJMewstU0YmdnqUJqm88iP6yaxMxWDixk2ckDuzMD/x6u/JBk+S8Fpt7+4pxbvGVIsqyRpImkRNJTUlY6vcCb3YEtsAY2/sEgEIwLaAn0azMM140l3STNi/1Y0i4U/MSqFQJGks51mA+Gts1XOJdTXaocWeBfYAzsLWAVlgK0vaEPpsUEmLEmMCvGyYy1gUkI1tQ2GTAkH6kT8ACuXWtQto0PHoCZl3MEVlbw/+MF7jSomC+qyG4AAAAASUVORK5CYII=',
            status1hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAA1UlEQVQ4jd3UMU5CQRDG8Z9oZaiILQ0xsaCx4ASQGBtLD2BoqIlXMKH3AJ6ChJ4aGioTOlv1DBTvkUweCLypDF+1k9n9z7c7meW/62K7+Jjd1Tm3RCfE65fHzx40kkYecI8prnG7TWSB3xiV4GFMXCWBkxLUx2VMZBxG2Ao/WGSBVRh8YZABtvBUge2ozhv+onts0ykOWzWKHgVOMK8DPHTl2ICTFR0ucbMH9mcD9ik67KCJ1yysCoQ3xTilYOw25RljxTi1M8DocK34Nd7LeCFMwPloAySBIA+0C1zpAAAAAElFTkSuQmCC',
            status2: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAA1klEQVQ4jdWTMQrCMBSGv5aiBe+gnsVJUNDNoQ4exkM4Oznp4OJgaw/jDRQHl9/BVEIsStKpPzwSkvDxv/fyoDWSFBqZpDwUOJTUdWA3SaNQYCFp6sIkfTixZ2ViIAEyYAPMgKJJDUtJO9tZFaEOARbAHugDSyC1LxNP2NGsA2AFPIESuFYPompj2w5RFL1RISn/lJtyDzgAHevsDKy9yVbHxpLmppsynf37AyrVpXwyTifA1tdYHdD+tJemQHcCHsDdFwrwNZsmIkmpTw1tYO6Ok0+0Ry8tQnBqYlIS0QAAAABJRU5ErkJggg==',
            status2hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAA20lEQVQ4jc3TsUqDMRiF4cdSpNB7sG69DIdaEDo4VtCh9lpcFcdOLbgKCnbpUKq34S7oBaioiw79C+FvWky69EBISPK9nEPyse3aWSxGk2Yu4xT986PnFlQTi/fxiu8ANsDx4kIlEThEOwKb5QIr5qmiMNIjwwk6MViOQ+jiDns4Qy08THU4LuYGevjBE15ygZfFWKmcyGtVdljHPXaDvSku/gssO/zAFa7xhgMcpjiMRZ4UTju4SYGtAoaf9nFTYLkDPvGeAgwfJdZOt3jIddi33E6/+EoBbr/+AEh5KFS9txb9AAAAAElFTkSuQmCC'
        });
        v.init();
        v.m_ele.id = "viewshareplayerflsn";
        v.m_ele.m_container = this;
        v.m_ele.style.right = "20px";
        this.controlbar.appendChild(v.m_ele);
        if (typeof this.nofullscreen == "boolean" && this.nofullscreen) {
            v.m_ele.style.display = "none";
        }

        // logo
        v = new ImageButton({
            width: 145,
            height: 40,
            // videolibrary
            //status1: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJEAAAAoCAYAAAD+HRieAAANQ0lEQVR4nO2ceZBVxRXGf7OwM4qCDGAhQlwAETWuqISIlBuipUaNRI27aFQkSoylptSKBE2icUErGrdywQUq4oaCGyq4IaJIkKCDgojCAAoKwzJ8+eN0c/vd1+/NY2aMo/W+qq77bve5p5d7us/p0+e+EgBJNADdgb2Aw4CeQAVQCtQC3wHzgdeBqcBHwLqGVFZE00MJ1FuI2gLHAScBfYBt89AuA6qAp4AHMcEq4ieC+ghRCXA8cBnw83rU+RVwB3ATsLIezxfRxLC5QrQd9vKPbYS63wSGAe83Aq8ifkBsjhD1B+4DejRi/csxdTipEXkW8X9GoUJ0KDAeaFMAz3VADbAW2AiUYfZTyxz0q4FjKArSjxaFCNEATIDa56GZDcwC3sHU0wLgG2ADtlPridlRvwK6RJ6fDxwIfLFZrS+iSaAuIeqObc075yh/GHgGGEdhW/duwLXAqZGyScCQAvkU0YSQT4jaAS8Du0fKPgT+iAmQR3PgCBKBW5AqD3EocDvZ9tUZwL0FtLuIpgZJsXST4nhSUseAbldJ/5T0kaTagG6dpPclXSSpLMK/q6TXU7znSNoqR3uKSWojqVkTaEdGyiVE+yuOxyS1cjQdJI2WtCYHbYiJknpE6ukh6bMU7QmN2MGukkoKoGsmqZ+kzo1Yd2OnvpI+kfSQ4pOySQlRqaQZysarkto6mv6SPozQ5MNCSYdFGjEkRTdehb34utJZstVxjKTWddCeJmm1bGXcuhHqLiSdIOlWSbsUSH+UG58vZBP4Bxcen2JCdKmysVDSNq78dElrIzSSVC1pvqRlOco3Sjol0pCnA5olshWkkA50DtoVpkrZrPU4ow4+dwa0FxZYd0PSIUrG8CVJWxTwzHGOfrGkbRtYfz+ZAD8jaZQaaEKkhaiLa2Qap7ry8yNly2SzfXdJW0qqcGk/ma2UVne1kgbkGCCPYwto/P4ygZ0heylh2YkpflfIBO4ySb0ivG4MaK8uoO6GpBYy9R6OR2yFLpON++nu/jeOfr7qr3ZbSLpB0jep8bm+nvwGSDowLURXKhvTXNngSNmTSpbjVpL2lXS0pD4Bz0HKtntekqlNT7NTqmOjC+jAsIB+pkyAfdl1qfqOdO2SzNBP8wr7fUkBdTck/UxSVVDfBpmApOnay1RXdaq/s5R75egom7wD3DWt9kYqjo8D2k7u2V+4a/vg+UpJA93vrpIWSfpPKESdJH0eqeAgmeGZLrs2YD5UNkNCjFNiQx0g2615rA0agxuU0A57RHW/jKGp+oa6/BJJ9wf5a2TCva2kayTtHOEVrrB/CPIrJG0neznbyMaooULUR5mTapVyr0S/lXSyu7/B0U/MwXcfSe/KbLs1kla6+/OVrEKzg3pvl3SP+71MtsnZQdJ7jsfqgMdZjsefHf3vJO3tfn9bGuz0f012OMdHwKvAFamyq4A/Yd7oO4CHgO1Tzx4HXO5+T3V0Hs0xT7jHt2R6q7fMdkRkYTbwdXDf010rgK5B/qfAHKATdgSzKsWnDTAouF/t6IYDLwIvAJMxZ+grwMiAtiVwHfAocBqwRQHtboH132MRNs5plLm+rMT8eR1dfgVQGaE/G4uqaOXaVeHuxwAXYUdQM7FxOwk4P6j3SyxUZxjmF2zlkudxF3B40L+rMacxWMwYyPwPbysbwyWVy5ZUj0cD6b8z8kyI+UoM30Gpsn8HfMolPRWUTVbdM7pUtmP0eMLld5E0N8j/i2x1mubuLw54HCDphVS7hko6OE+fvHpvJXN5hJisujcFAyQtD54Zm4PO24lzZWPo66p1eWNkKwcyw/y9PG2eIam5TKNsL9MuY5UY9xMltVS23y7E7TL7LI2F5U6aegN7R6T7DeBgknMzAde436c76fdYjK1auzp+uNnTDliKBaatctIN5tlujc381uQ+WsmFjcAMLLoALLqyCzYLtw7oxgO7Af3cfWt3vQS40rUvhMj0pG8A3gLmufvHsRX4Nuw8MMQgbJbfkKfdbYBmwf1LEZoS4ET3uxY7wG7r7kuBnVwaAhziaHrlqXMydpzUEbgVODJVvhQ7kto1D49ZwGvYyhMexK/y6uygyENzseONQ4O8GS6/DLg0yH8SGIipxCEkKmMj9lLAXsba4JmtSF5gJ2DHoCyky4dXAv7bAh2wwfZ8Z2GqLOxDFRY18DeyBQjsBW5I3W+NqdhrgWeBo7EjGo83sHEBm4zl5EYFiTqrxl5MGpXAnu53DYlqAzvYvgVTU50xVdqLbDXt8RZwPSZ8Y8kWIID1WJ9ztfsdbPJ8jMWBhZjthWgQ2XgbWyV2S+XVYnrSrzZzMeHx+rUZ7kwOs3W8QLQkWQXABtLPyN5k2hPVOToTa+PHwf1FwAiSwfgQWBO0tda1w9s1813bRwQ85Og8yrCXdDjJixwSlI8BjsLsCrAJEdo8abQj6Xc18b52I1mZa7DxbOXu78HstQuACUBf1+aXI3y+Bi7G4rauxCb6BmAUmTZqM+Az4gGCS7FV27fzPmxx8HjDC9EOkYe/ctfQoF7grgODvAnYiwKT9r+TLL0LsZkDNpNDIfoOWOF+p2fHnEh7YvgCWwU8zsQMXI/PsQ57Q3QpZoDv4u5fxmbysuCZcrJn9SRMfS9x5d7wX4WtTt2BAwLamjxt3oZkkq0I6m7h+HgaLzSLMCHxKuRrbJwHAfthk3QeMD1S1wps5ajEjGGPvYB9g/tVmHC9G+FRjRnkHuNSdJ94IYoFm/nZGOpvvxsKdWd/d98Xsz8GB2VTSeKo0zuuT11ZB0y9hPW+GmlPLryep8zbMV6o/YB4dXUUtlu5O3imFZmx38I+RLjF8etLMsFaYFEH4zDhehNbmcKZmkZo+y3EVq4jgecwtXGbo/E8PsLGZCt3PwxT4+OxCb7E8Vkf8PVC3B3bIa8nmczlmB0VxsdPdddwMn3nrr0wdRjy9qtYNVDll/2YLvSRiOGA+MEPha4f8EHkecg0Gg9OlfmQj6vJNIRn5+EXw2vYIHaNlPmvSvwKWIvZb92A0ZgAd3BlqzB76RVMcNZhaqkEM9jXYKve5yQrsg9/8c+fh6mOXCgjWW0AfglMIdMePB24mcTWW4BN3jJ335lMQZyLvfCqIO9G19ersI3QImyFHu76swwTjt6YED7tngs1wM2YOTLK9asKsyMhWfWqMDUIkpZGtm4vuO3ji0HeOS7v7DxbQY/wwLa7zHnl8biSLXb6HG6kNs95Vy7p4Uj9NZL2VKYrYlTw3GWS7pI5TU+RnSd572y5pL/KDnDHytwC+7h875h9NqhrvcwxWFdbe8nOIfPhAUl7yBySC5V4/y+O0E6XHTchO2R+SjbOB7q8fwW0nk9Ld+0m6UFJI4L2tZVt99fI3ABIujvg0c/lneDu75eSs7OYp3q97DjinCDvJiXe1NiL85glacegcY8HZfNkPpY2spcUYpHMz7O5XuDTIm2okvlEkHnE91YygIWksjra0lwmOJfJohoK4enPv9JY79p7uZJQm52V+IF8Ol7SzTKv+hHKPv7YQiYsvp8lkiY43nsV2MatJO0W8Gge8NjX5V3r2j1SgRBNz9G5yx0zL2RfKtOZNlzmfJsn6b+SpspmcGVAE0YFrFUyS+6K1HdpgR2NzfD0RHjWDUB9+H1f6ZJIn9+WOWLbfE91lioRzMbiMd61/XgFQjQ60jlJWiGpnZLlS5KeU3bMzZaKHwqOUOaZ2bku/7wcg+nV3+amMpknPcQ1DRy47yONSLXxAzXtQLhYaiM7sN0UjeGFKFckoyRd4B5+PsibqSREIZ3aSzpTmccYcvfIwjZqU2UrJPVuYOfODfh9K7NhfugBT6d+SqIVpqnwuKmmlHrKIg8Wy8ydTUJULmmK4pjpHvZH/yFmyPTl3TIj63mZlKYxT7Za9JQJTBreYG9IqnR9qFGmAd3U0kBZkFxFE2hLfZI/05sqp+K8EKHsQC6PGiVW+WBlBzTVhWolRt07kfJ/NGIHO8h2ZOWNyLOYMpMPB3nY54WhIE8Qd523wMIGwD4BOgzzbRSCKZh/aDowEfOUhngE+H2BvApBNeZN3VAXYRH1Rh93XZyRG0jZHrJApDRqZDsIT1cqC5l4TLb1WynbeS2XBfDfJ1vZkG0z0/aRZGqwyX3+Ukx5U4USt8wm/1JaiFA8RFYyNTZQ2Yw7y/wZO8ui48Jwyh4y3ZnGvWpin70UU0HpSNnHFlIQBx8TojJlemND1MhsmLp2Ui1kYZ1LIjyuawKDUUz1S/e6d/iNbEePpJyfUVcC75E7UEzYP55Nws5PFmMny12xw9iTgX1Sz6zAzm4e2Az9W0TTQS9gGhbK8hUWgzYH8n+L3x8LwGobKwywDgvJKMX+BCuG17BYn5k5yoto+rgQi2QAizg4BhctUJrrCezFn0/d/9LRHAvSjwnQ11hA/+EUBejHjDKSDytWYqf5m2Km8oVxgqme1cCfSb6mKATLgeexgK3YlwxF/LhQQRKuMgH7CmYT6hIisOCnKVgw1GAsmi6G1VhQ1VQsHre48vx0sJZk5XkwXfg/MtCIQoQ7GrAAAAAASUVORK5CYII='
            // smartbox
            //status1: 'data:image/png;base64,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'
            // bbc a certain project
            status1: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJEAAAAyCAMAAABrjslRAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAC31BMVEUAAAD7/f37/f37/f37/f37/f37/f37/f37/f37/f3////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7/f37/f3////////////////////////////////////////////////////////////////////////////////7/f37/f3////////////////////////////////////////////////////////////////////////////////7/f3////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7/f3///8AAADgIa/KAAAA8nRSTlMAETOZu3dEqmYiBA8bIxALJykKAg0HUc1CFmU7KoHQBVz6oCElMkFISUABE6mQH0ytHaOx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'
        });
        v.init();
        v.m_ele.style.right = "-32px";
        v.m_ele.style.top = "40px";
        v.m_ele.style.cursor = "default";
        v.m_ele.removeEventListener('mouseenter', onPMouseEnter);
        this.uilayer.appendChild(v.m_ele);

        // volume
        this.volumebutton = new ImageButton({
            width: 20,
            height: 20,
            status1: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAABQklEQVQ4jbWUPy+DURSHHzSpGBqJRDBQjc1CooPBYCAxmUh8A/EVLBZTYzKQ2CUWo93I2kSaVGglEgsVMRgaj6Ett9U/b6N+yW+47733ueec99wL/yE1qofUQ3Ws2Xy3wCU1b0XJVsD+CAkMAhngErj7a8ppNac+q5vVyBojTKnxxpTj6lyD99WyeqFOVDc3A96oe43AA3/rXd1W+4LNzYAZtaD2hzUcAa6A6cCTwDEQHN1Up8AUMAsQCyaegEKkotcrC3wA80A2yl/upDJwC6QgWttE0RuQ6CVwGChBfQ1HgWQwfq26k2JU0i3CT4QlYBG4D1wAtiIA01Ru03UY4S5wDgwECzeotMQ6sAO8tACuAXkg9/2lzdVbVovqo7rSorFn1IVuXpuEeqJ+qmedXpsowJpX1YdeAmvRHqnjbYG91BeZeCEdGDt30AAAAABJRU5ErkJggg==',
            status1hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAABZklEQVQ4ja3UsUscQRjG4efCgZJCQgKWJtpEiRYWFhYKSaP+AYJ2hyA2VmlSpBHBgIi9Vh6CISSpgoJno2KllcHmQInaSDqDEFEQk2JcXI/1dg994WOZ5Z3ffjP7zvDIysHC6uus/qeYxhR+Ry8LfeUAy+U8qeHjPfiJcdTfZ8oCrMcMNvErzZwG7MIuRjCEsQRPS7HUWhcN8jfPOrRVGAfxAWt4hxO8SgAu4ysm4sBPeF9h/Cvs1zz+VVnFCgrFUuskrqMlv8A2mmPVhLkUGHzGS7yJd0iIwVHK5CTt4QKd2KslNvfpCgdoIVtssugMDY8JfIZT7u5ho7ux+HNTacoLyz3mtsNTdOMwVkcYzgDsEk7TThz4Eb14G6slIRJf8LwKcAD7hb5yOWoXzrFVYdzAdxSFaBSwnwBcxI9okE8wxLWODsyihG8JnoPo+iLbXz7DKPqFfa6qWmKzhnbhOF7WMO9h+g8ajkbXKmwXTAAAAABJRU5ErkJggg==',
            status2: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAABTUlEQVQ4jbWTvSuFYRjGfzhFBgMltrOKATFQCsVkJVZRyh9g8hdIBl8DsjJJyWAzSRkNSglFFpLM52c4z1uPj/N6dY6rrrqf3vu57ut+ux74D6hZWa+uqi2lev4iOKBeW0S+VF91hgXqgCXgFLgpd+Ve9Up9VieCs1SHuaBZC7R9mTMOLAAnwDDwCOSzOlz2O97VObUqchA7PFC7om+d6kHisAk4ByajOW/AS4qPY2AD6A/nTWA3FzU8AbeZVipiB5gBpgGBGmA7l3olHQVgHjgK5zGgUI4gwAVwGdWZcpiGHqAd6Ag1scNmPsfiNbAUqoE1YJHiP1wH+gBQV36Izas69SW4cWxm1LMQq6pQzyaC9eGtDkZcD5f31MYfBA/V7mhYl3qYCJbikHqnPqgjWZ7eb4KoDeqWWlD3KyGYcFS9r6Rg4nZTbU0VrCQ+AD9XfHUlSRuOAAAAAElFTkSuQmCC',
            status2hover: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAABdklEQVQ4ja3UP0hVYRgG8F8qKI6GzTZ1RYMGGwwSbLkNOga2HVIXc2ppCAkCg2i1f5DdA5G0uYh0XKSxEERwuBCYkzilCA0tXofvHrlerueey+2Bdzjnfb/ne9/nffj4z7gCpW838tb34hUWcZj+jIrl84KOFi6/ix3Mo+eyojyEPXiN79hrVtzVJH8bn9GPKfzE7zyE3Risyz3AU2zgHg4wkLfDl3hSl/sr6PUBlQZnV/EC2xAnhVt4nhJexQ9hrBQn+JPRzDre4k6cFOAdSrUaHmI/a5w6LGMGj6oTdOJjs6Vk4RSPsVb9noiK5dN2CGELuxAVy1u0ZuxGGMEQhuOkMMJFH15z0RbH1bgMHVjCgqDhmzgpjKYdHmFUMG0a+3iYQZguYxmfBE2nU8JnGMN4TXzBCr6irwHhpLCUSlQsVzCHiWavzThiQZoIv6rdX1djsVZem03cFEycCE9XJvJs+QSzuC/o3DZhig0M4z3+tXCuPZwB+TNUKl7jp0AAAAAASUVORK5CYII='
        });
        this.volumebutton.init();
        this.volumebutton.m_ele.m_container = this;
        this.volumebutton.m_ele.id = 'viewshareplayervolume';
        this.volumebutton.m_ele.style.right = "70px";
        this.controlbar.appendChild(this.volumebutton.m_ele);

        // subtitle
        if (typeof this.subtitle == 'object') {
            v =0;
            if (typeof this.subtitle['zh'] == 'string' && this.subtitle['zh'].length > 10) {
                v += 1;
            } else {
                this.subtitle['zh'] = null;
            }
            if (typeof this.subtitle['en'] == 'string' && this.subtitle['en'].length > 10) {
                v += 1;
            } else {
                this.subtitle['en'] = null;
            }
            if (typeof this.subtitle['en&zh-cn'] == 'string' && this.subtitle['en&zh-cn'].length > 10) {
                v += 1;
            } else {
                this.subtitle['en&zh-cn'] = null;
            }

            if (v > 0) {
                console.log("subtitle exist");
                this.subtitle.subs_number = v;
                v = new ImageButton({
                    width: 35,
                    height: 20,
                    status1: "rgba(0,0,0,0)",
                    dontlistenmouse: true
                });
                v.init();
                v.m_ele.id = "viewshareplayersubtitle";
                v.m_ele.innerHTML = "字幕";
                v.m_ele.style.right = "110px";
                v.m_ele.style.fontFamily = "SegoeUI";
                v.m_ele.style.fontSize = "14px";
                v.m_ele.style.color = "#ffffff";
                v.m_ele.m_container = this;
                this.controlbar.appendChild(v.m_ele);

                this.subtitlebar = new PlaySubtitleShow({
                    subtitle: this.subtitle,
                    right: 89,
                    bottom: 50,
                    parent: this.uilayer
                });
                this.subtitlebar.init();
                this.uilayer.appendChild(this.subtitlebar.m_ele);
            }
        }
    }
    PlayerSkin.prototype.clean = function () {
        console.error('skin destroy');
        try{
            if (window.localStorage && this.progressbar.m_currentTime > 2) {
                localStorage.setItem(this.uri, this.progressbar.m_currentTime);
            }
            this.ele.parentNode.removeChild(this.uilayer);
            window.m_vst1 = null;
            if (typeof this.oldstyle_record_for_fit_sar_not_1 == 'string') {
                this.ele.style = JSON.parse(this.oldstyle_record_for_fit_sar_not_1);
                this.oldstyle_record_for_fit_sar_not_1 = 0;
            }
        }catch (err) {
            console.log(err);
        }
    }
    PlayerSkin.prototype.handleMouseUp = function (e) {
        this.progressbar.handleMouseUp(e);
    }
    PlayerSkin.prototype.handleMouseMove = function (e) {
        if (this.progressbar.m_bcursorclick) {
            this.progressbar.handleMouseMove(e);
        } else {
            if (!this.uishow) {
                this.setSkinVisible(true);
            }
        }

        if (this.volumebar) {
            if (this.volumebar.visible) {
                // volume button size 20x20
                if (e.pageX <= this.volumebar.mouseposinpagex_whenclickvolume - 30 ||
                    e.pageX >= this.volumebar.mouseposinpagex_whenclickvolume + 30 ||
                    e.pageY >= this.volumebar.mouseposinpagey_whenclickvolume + 20) {
                    this.volumebar.setVisible(false);
                }

                this.stoppos.t = new Date().getTime() + 36000 * 1000;
                return;
            }
        }

        if (this.subtitlebar && this.subtitlebar.visible) {
            // volume button size 20x20
            if (e.pageX <= this.subtitlebar.mouseposinpagex_whenclickvolume - 30 ||
                e.pageX >= this.subtitlebar.mouseposinpagex_whenclickvolume + 30 ||
                e.pageY >= this.subtitlebar.mouseposinpagey_whenclickvolume + 20) {
                this.subtitlebar.setVisible(false);
            }

            this.stoppos.t = new Date().getTime() + 36000 * 1000;
            return;
        }

        if (e.target != e.currentTarget) {
            this.stoppos.t = new Date().getTime() + 60 * 1000;
        } else {
            this.stoppos.t = new Date().getTime();
        }

    }
    PlayerSkin.prototype.handleMouseEnter = function (e) {
        this.setSkinVisible(true);
        document.addEventListener('keypress', onPKeyDown);
    }
    PlayerSkin.prototype.handleMouseLeave = function (e) {
        //console.log('mouse leave');
        this.setSkinVisible(false, 2000);
        document.removeEventListener('keypress', onPKeyDown);
    }
    PlayerSkin.prototype.setSkinVisible = function (b) {
        clearTimeout(this.tid);

        var o = this;
        var t = b ? 200 : (arguments.length == 2 ? arguments[1] : 1500);
        this.tid = setTimeout(function () {
            setVisibleG(b, o);
        }, t);

        this.uishow = b;
        this.stoppos.t = new Date().getTime();
        this.setCheckMouseHoverForAWhile(b);
    }
    PlayerSkin.prototype.setCheckMouseHoverForAWhile = function (needcheck) {
        clearTimeout(this.tid1);
        if (needcheck && this.uishow) {
            var o = this;
            this.tid1 = setTimeout(function () {
                checkMouseHover(o);
            }, 1000);
        }
    }
    function setVisibleG(b, o) {
        //console.log('setVisibleG');
        if (b) {
            o.controlbar.style.visibility = "visible";
            o.progressbar.setVisible(true);
        } else {
            o.controlbar.style.visibility = "hidden";
            o.progressbar.setVisible(false);
            if (o.volumebar) {
                o.volumebar.setVisible(false);
            }
            if (o.subtitlebar) {
                o.subtitlebar.setVisible(false);
            }
        }
    }
    function checkMouseHover(o) {
        var now = new Date();
        // 2-3 seconds to hide
        if (now.getTime() - o.stoppos.t > 2000) {
            //console.log(o);
            o.setSkinVisible(false, 1000);
            //setVisibleG(true, o);
        } else {
            o.setCheckMouseHoverForAWhile(true);
        }
    }
    PlayerSkin.prototype.startPlay = function () {
        if (this.vinit) {
            if (window.vs_haltplay != true)
            this.ele.play();
            return true;
        }
        return false;
    }
    PlayerSkin.prototype.pausePlay = function () {
        if (this.vinit) {
            //console.log('pause play');
            this.ele.pause();
            return true;
        }
        return false;
    }
    PlayerSkin.prototype.notifyVideo = function (pp) {
        if (pp.type == 'update') {
            var i = 0;
            var t = this.ele.currentTime;
            for (; i < pp.tr.length; i += 2) {
                if (t >= pp.tr[i] && t < pp.tr[i + 1]) {
                    this.progressbar.updateBuffedTime(t, pp.tr[i + 1]);
                    return;
                }
            }

            this.progressbar.updateBuffedTime(t, t + pp.tr[1] - pp.tr[0]);
        } else if (pp.type == 'init') {
            this.vinit = true;
            this.vduration = pp.duration;
            console.log('vinit ' + pp.duration);

            this.progressbar.m_currentTime = -1.0;
            this.progressbar.m_duration = pp.duration;
            this.ele.m_customcb = this;
            this.ele.addEventListener('timeupdate', onPTimeUpdateon);

            var alltimespan = this.showtime.lastChild
            alltimespan.innerHTML = " / " + formatTime(pp.duration, false);

            if (typeof this.limittime != 'number') {
                this.limittime = -1;
            } else
                window.vs_haltplaytime = this.limittime / this.vduration * 100;

            // 现在浏览器普遍不允许不是由用户操作触发的play，所以下面这一段代码难以起作用
            if (typeof this.autoplay == 'boolean' && this.autoplay) {

                var v = document.getElementById('plorpa');
                if (v && v.m_customcb && v.m_container) {

                    v.m_customcb.handleClick({
                        target: {
                            id: 'plorpa'
                        }
                    }, v.m_container);
                }

            }

            if (window.localStorage && this.limittime <= 0) {
                if (localStorage.getItem(this.uri) != null) {
                    var layer = this.uilayer;
                    var t = localStorage.getItem(this.uri);

                    setTimeout(function () {
                        var v = document.createElement('label');
                        v.setAttribute('style', 'position:absolute; height:20px; line-height:20px; left:0; bottom: 55px; text-align:left; color:#cccccc; background-color:rgba(0,0,0,0.7);');
                        v.innerHTML = "上次观看到 " + formatTime(t, true) + " , " + "<span id='viewshareplayer_skip_to_last_play'; style='color:#ff8844;';>继续观看？</span>";
                        layer.appendChild(v);

                        v = document.getElementById('viewshareplayer_skip_to_last_play');
                        if (v) {
                            v.m_customcb = {
                                handleClick: function () {
                                    this.skin.seekTime(this.time / this.skin.vduration * 100);
                                    this.ele.style.visibility = 'hidden';
                                },
                                skin: layer.m_customcb,
                                time: t,
                                ele: v.parentNode
                            };
                            v.addEventListener('click', onPClick);
                        }
                    }, 2333);

                    setTimeout(function () {
                        var v = document.getElementById('viewshareplayer_skip_to_last_play');
                        if (v) {
                            v.parentNode.style.visibility = 'hidden';
                        }
                    }, 6666);
                }
            }
        } else if (pp.type == 'play') {
            v = document.getElementById('plorpa');
            v.m_customcb.changeStatus(true);
            if (this.videoposter)
                this.videoposter.style.display = "none";
        } else if (pp.type == 'pause') {
            v = document.getElementById('plorpa');
            v.m_customcb.changeStatus(false);
        } else if (pp.type == 'checksarforwebkit') {
            // webkit browser sar issue
            if (window.ActiveXObject || "ActiveXObject" in window || navigator.userAgent.indexOf("Edge") != -1) {
                console.log("iedge");
            }
            else if (navigator.userAgent.indexOf("Firefox") != -1) {
                console.log("ff");
            }
            else if (!!window.chrome) {
                console.log('is chrome');
                if (navigator.userAgent.indexOf('OPR') != -1) {
                    console.log("opera");
                } else {
                    i = Module.ccall("_Z2akv", 'string', []);
                    i = i.split('-');
                    if (i[2] != i[3] && i[2] != 0) {
                        function calVideoPosition(i, rar) { // represent aspect ratio
                            var dar = i[0] / i[1] * i[2] / i[3];
                            this.oldstyle_record_for_fit_sar_not_1 = JSON.stringify(this.ele.style);
                            this.sar_frame_info = {
                                ia: i,
                                rar: rar
                            };
                            console.log("dar " + dar + " rar " + rar + " sar " + (i[2] / i[3]));
                            //document.getElementById("info").innerHTML = "dar " + dar + " rar " + rar + " sar " + (i[2] / i[3]);

                            i = dar / rar;
                            if (i >= 1) {
                                this.ele.style.width = "100%";
                                this.ele.style.height = new String(100 / i) + "%";
                                this.ele.style.left = "0%";
                                this.ele.style.top = new String(100 * (1 - 1 / i) / 2) + "%";
                            } else {
                                this.ele.style.height = "100%";
                                this.ele.style.width = new String(100 * i) + "%";
                                this.ele.style.top = "0%";
                                this.ele.style.left = new String(100 * (1 - 1 * i) / 2) + "%";
                            }
                            this.ele.style.position = "absolute";
                            this.ele.style.objectFit = "fill";
                        }
                        calVideoPosition.call(this, i, this.ele.offsetWidth / this.ele.offsetHeight);

                        v = document.createElement('iframe');
                        v.setAttribute("style", "width:100%; height:100%; position:absolute; top:0; left:0;z-index:-100;");
                        v.style.visibility = "hidden";
                        this.uilayer.appendChild(v);
                        v.contentWindow.m_ghost = this;
                        v.contentWindow.addEventListener('resize', function (e) {
                            //console.log("iframe resize");
                            var ins = this.m_ghost;
                            setTimeout(function () {
                                var newrar = ins.uilayer.offsetWidth / ins.uilayer.offsetHeight;
                                if (Math.abs(newrar - ins.sar_frame_info.rar) >= 0.05) {
                                    calVideoPosition.call(ins, ins.sar_frame_info.ia, newrar);
                                }
                            }, 60);
                        });
                    }
                }
            }
        }
    }
    function onPTimeUpdateon(e) {
        var t = this.currentTime;
        var d = t - this.m_customcb.vduration;
        if (d > 0) {
            t = this.m_customcb.vduration;
        }
        if (Math.abs(t - this.m_customcb.progressbar.m_currentTime) > 0.75 || d > -1) {
            this.m_customcb.progressbar.updatePlayTime(t);
            this.m_customcb.showtime.firstChild.innerHTML = formatTime(t);

            if (this.m_customcb.subtitlebar)
                this.m_customcb.subtitlebar.updateSubtitle(t);

            if (this.m_customcb.limittime > 0) {
                if (t >= this.m_customcb.limittime) {
                    this.m_customcb.pausePlay();
                    pp.playcb("timeexceed");
                    window.vs_haltplay = true;
                } else
                    window.vs_haltplay = false;
            }
        }
        e.stopPropagation();
    }
    function formatTime(t, b) {
        var h = 0, m = 0, s = 0;
        var res = '';

        t = Math.round(t);
        if (t >= 3600) {
            h = Math.floor(t / 3600);
            t = t % 3600;

            if (h < 10) {
                res = res + '0' + h.toString();
            } else {
                res = h.toString();
            }
            res += ':';
        } else if (b) {
            res = "00:";
        }

        if (t >= 60) {
            m = Math.floor(t / 60);
            t = t % 60;
        }
        if (m < 10) {
            res = res + '0' + m.toString();
        } else {
            res = res + m.toString();
        }
        res += ':';

        s = t;
        if (s < 10) {
            res = res + '0' + s.toString();
        } else {
            res = res + s.toString();
        }

        return res;
    }
    PlayerSkin.prototype.seekTime = function (t) {
        if (this.vinit) {
            if (window.vs_haltplay != true )
                this.ele.currentTime = this.vduration * t / 100;
        }
    }
    PlayerSkin.prototype.enterFullSceen = function () {

        var pe = this.ele.parentNode;

        if (typeof window.m_vst1 == 'undefined' || !window.m_vst1) {
            window.m_vst1 = pe;
            pe.m_customcb = this;
            pe.m_isfullscreen = false;

            if (pe.requestFullscreen) {
                document.addEventListener('fullscreenchange', onFullscreenChange);
            } else if (pe.mozRequestFullScreen) {
                document.addEventListener('mozfullscreenchange', onFullscreenChange);
            } else if (pe.msRequestFullscreen) {
                document.addEventListener('MSFullscreenChange', onFullscreenChange);
            } else if (pe.webkitRequestFullscreen) {
                document.addEventListener('webkitfullscreenchange', onFullscreenChange);
            }
        }

        var s = document.getElementById('dynamicstyletagforviewshareplayerfullscreenfunctionid');
        if (!s) {
            var c = 'viewshareplayerclass' + Math.round(Math.random() * 500000);
            pe.className = pe.className + " " + c;
            var head = document.getElementsByTagName('head')[0];
            s = document.createElement('style');
            var sl = '.' + c;
            var st = '{width:100vw !important;height:100vh !important;left:0 !important;top:0 !important;background-color:black !important; position:fixed !important;}';
            s.setAttribute('type', 'text/css');
            s.setAttribute('id', 'dynamicstyletagforviewshareplayerfullscreenfunctionid');
            s.innerText = sl + ':-webkit-full-screen' + st +
                sl + ":-moz-full-screen " + st +
                sl + ":-ms-fullscreen " + st;
            head.appendChild(s);
        } else {
            s = s.innerText.substring(1, s.innerText.indexOf(':'));
            if (pe.className.indexOf(s) == -1) {
                pe.className = pe.className + " " + s;
            }
        }

        var fullscreenEnabled = document.fullscreenEnabled || document.mozFullScreenEnabled || document.webkitFullscreenEnabled || document.msFullscreenEnabled;
        if (!fullscreenEnabled) {
            return;
        }
        var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
        if (fullscreenElement) {
            return;
        }
        if (pe.requestFullscreen) {
            pe.requestFullscreen();
        } else if (pe.mozRequestFullScreen) {
            pe.mozRequestFullScreen();
        } else if (pe.msRequestFullscreen) {
            pe.msRequestFullscreen();
        } else if (pe.webkitRequestFullscreen) {
            pe.webkitRequestFullScreen();
        }

    }
    PlayerSkin.prototype.leaveFullScreen = function () {
        var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
        if (!fullscreenElement || fullscreenElement != this.ele.parentNode) {
            return;
        }
        if (document.exitFullscreen) {
            document.exitFullscreen();
        } else if (document.msExitFullscreen) {
            document.msExitFullscreen();
        } else if (document.mozCancelFullScreen) {
            document.mozCancelFullScreen();
        } else if (document.webkitExitFullscreen) {
            document.webkitExitFullscreen();
        }
    }
    PlayerSkin.prototype.switchFullScreenButtonStatus = function (fe) {

        var b = document.getElementById('viewshareplayerflsn');
        if (fe) {
            // enter fullscreen
            b.m_customcb.changeStatus(true);
            b.m_customcb.setBg(false);
            window.m_vst1.m_isfullscreen = true;
        } else {
            // leave fullscreen
            b.m_customcb.changeStatus(false);
            b.m_customcb.setBg(false);
            window.m_vst1.m_isfullscreen = false;
        }
    }
    function onFullscreenChange(e) {
        var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;

        if (fullscreenElement) {
            if (fullscreenElement.m_customcb) {
                fullscreenElement.m_customcb.switchFullScreenButtonStatus(fullscreenElement);
            }
        } else {
            if (window.m_vst1 && window.m_vst1.m_customcb && window.m_vst1.m_isfullscreen) {
                window.m_vst1.m_customcb.switchFullScreenButtonStatus(null);
            }
        }
    }
    PlayerSkin.prototype.showVolumeBar = function (b) {
        if (this.volumebar == null) {
            this.volumebar = new PlayVolumeShow({
                right: 70,
                bottom: 50
            });
            this.volumebar.init();
            this.volumebar.m_container = this;
            this.uilayer.appendChild(this.volumebar.m_ele);
        }

        this.volumebar.recordMouseInpage(arguments[1].pageX, arguments[1].pageY);
        if (!this.volumebar.visible){
            this.volumebar.setVisible(true);
        }
        else {
            this.volumebar.setVisible(false);
            //this.volumebar.setCursorPop(0);
            //this.setVolume(0);
        }
    }
    PlayerSkin.prototype.setVolume = function (v) {
        if (this.vinit) {

            if (v < 0.01) {
                v = 0;
                if (!this.volumebutton.m_switch) {
                    this.volumebutton.changeStatus(true);
                    this.volumebutton.setBg(false);
                }
            } else {
                if (this.volumebutton.m_switch) {
                    this.volumebutton.changeStatus(false);
                    this.volumebutton.setBg(false);
                }
            }

            this.ele.volume = v;
        }
    }
    PlayerSkin.prototype.showSubtitleBar = function (b) {

        this.subtitlebar.setVisible(!this.subtitlebar.visible);
        this.subtitlebar.mouseposinpagex_whenclickvolume = arguments[1].pageX;
        this.subtitlebar.mouseposinpagey_whenclickvolume = arguments[1].pageY;
    }

    return new PlayerSkin();
}