async function refresh_jobs() {
    let rows = JSON.parse(await http_get('jobs', {})).result;

    // Modify fields as needed
    // Change "app link" to link TODO
    // Add link to "status" TODO
    // Add link to "progress" TODO
    // Add link to "interview details" TODO

    // Clear out table
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

window.onload = async function() {
    while (true) {
        await refresh_jobs();
        await sleep(60 * 1000);
    }
};
