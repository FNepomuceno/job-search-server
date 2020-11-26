let request_button = document.getElementById("job_request");

function handle_response() {
    let request = this;
    if (request.status != 200) { return; }

    let json_response = JSON.parse(request.responseText);
    let rows = json_response.result;

    // Modify fields as needed
    // Change "app link" to link TODO
    // Add link to "status" TODO
    // Add link to "progress" TODO
    // Add link to "interview details" TODO

    // Clear out table TODO
    let table = document.getElementById("jobs_table");
    while (table.rows.length > 1) { table.deleteRow(1); }

    // Add result rows
    if (rows.length) {
        for (let row of rows) {
            let new_row = table.insertRow();
            for (let value of Object.values(row)) {
                let new_cell = new_row.insertCell();
                new_cell.innerHTML = value;
            }
        }
    } else {
        let new_row = table.insertRow();
        let new_cell = new_row.insertCell();
        new_cell.innerHTML = "No applications found";
        new_cell.colSpan = 12;
    }
}

function refresh_jobs() {
    let httpRequest = new XMLHttpRequest();
    httpRequest.open('GET', 'jobs');
    httpRequest.onload = handle_response;
    httpRequest.send();
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

window.onload = async function() {
    while (true) {
        refresh_jobs();
        await sleep(60 * 1000);
        break;
    }
};
