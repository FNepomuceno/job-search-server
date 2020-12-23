function param_string(map) {
    let entry_strings = [];
    for (const [key, value] of Object.entries(map)) {
        entry_strings.push(`${key}=${encodeURIComponent(value)}`);
    }

    if (entry_strings.length < 1) { return ''; }
    else { return '?' + entry_strings.join('&'); }
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
