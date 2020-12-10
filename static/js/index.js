async function refresh_jobs() {
    let rows = JSON.parse(await http_get('/api/jobs', {})).result;

    rows.forEach((row) => {
        // Change "app link" to link
        let app_link = document.createElement('a');
        app_link.href = row['app_link'];
        app_link.innerHTML = 'Link';
        row['app_link'] = app_link;

        // Change interview details to edit
        let edit_link = document.createElement('a');
        let row_id = row['date_applied'].split(/[-: ]/).join('-');
        edit_link.href = `/jobs/${row_id}`;
        edit_link.innerHTML = 'Edit';
        row['interview_details'] = edit_link;
    });

    // Clear out table
    let table = document.getElementById("jobs_table");
    while (table.rows.length > 1) { table.deleteRow(1); }

    // Add result rows
    if (rows.length) {
        for (let row of rows) {
            let new_row = table.insertRow();
            for (let value of Object.values(row)) {
                let new_cell = new_row.insertCell();
                if (typeof value == "string") {
                    value = document.createTextNode(value);
                }
                new_cell.appendChild(value);
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
}

window.onload = async function() {
    while (true) {
        await refresh_jobs();
        await sleep(60 * 1000);
    }
};
