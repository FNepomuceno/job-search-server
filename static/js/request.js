function param_string(map) {
    return '';
}

function http_get(url, params) {
    return new Promise((res, rej) => {
        let xhr = new XMLHttpRequest();
        let p_str = param_string(params);
        xhr.open('GET', `${url}${p_str}`);
        xhr.onload = function() {
            if (this.readyState != 4) { return; }

            if (this.status == 200) { res(this.responseText); }
            else { rej(this.responseText); }
        }
        xhr.send();
    });
}

function http_post(url, params) {
    return new Promise((res, rej) => {
        let xhr = new XMLHttpRequest();
        let p_str = param_string(params);
        xhr.open('POST', `${url}${p_str}`);
        xhr.onload = function() {
            if (this.readyState != 4) { return; }

            if (this.status == 200) { res(this.responseText); }
            else { rej(this.responseText); }
        }
        xhr.send();
    });
}

function sleep(ms) {
    return new Promise(res => setTimeout(res, ms));
}