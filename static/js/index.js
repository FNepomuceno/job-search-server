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

    // Update "last updated" text
    let update_text = document.getElementById("last_updated");
    let options = {
        year: 'numeric',
        month: 'long',
        day: 'numeric',
        hour: 'numeric',
        minute: '2-digit'
    };
    let date_string = (new Date()).toLocaleDateString("en-US", options);
    update_text.innerHTML = `Last updated: ${date_string}`;
    console.log(update_text);
}

window.onload = async function() {
    while (true) {
        await refresh_jobs();
        await sleep(60 * 1000);
    }
};
