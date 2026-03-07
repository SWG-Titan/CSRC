	<Page
		BackgroundColor='#000000'
		BackgroundOpacity='0.70'
		Name='CalendarSystem'
		PackSize='1,1'
		ScrollExtent='1024,768'
		Size='1024,768'
		Visible='false'
		VisualEditLock='true'
	>
		<Page
			BackgroundColor='#404040'
			BackgroundOpacity='0.95'
			BackgroundTint='#00354F'
			Location='112,84'
			MinimumSize='750,550'
			Name='calendarWindow'
			PalBgTint='back1'
			ScrollExtent='750,550'
			Size='750,550'
			UserMovable='true'
			UserResizable='false'
		>
			<Data
				Name='CodeData'
				btnClose='btnClose'
				header.btnPrevMonth='header.btnPrevMonth'
				header.btnNextMonth='header.btnNextMonth'
				header.btnToday='header.btnToday'
				header.btnCreate='header.btnCreate'
				header.btnSettings='header.btnSettings'
				header.lblMonthYear='header.lblMonthYear'
				eventPanel.lblSelectedDate='eventPanel.lblSelectedDate'
				eventPanel.lstEvents='eventPanel.lstEvents'
				eventPanel.dataEvents='eventPanel.dataEvents'
				eventPanel.btnViewDetails='eventPanel.btnViewDetails'
				eventPanel.btnDeleteEvent='eventPanel.btnDeleteEvent'
				calendarGrid.day01='calendarGrid.day01'
				calendarGrid.day02='calendarGrid.day02'
				calendarGrid.day03='calendarGrid.day03'
				calendarGrid.day04='calendarGrid.day04'
				calendarGrid.day05='calendarGrid.day05'
				calendarGrid.day06='calendarGrid.day06'
				calendarGrid.day07='calendarGrid.day07'
				calendarGrid.day08='calendarGrid.day08'
				calendarGrid.day09='calendarGrid.day09'
				calendarGrid.day10='calendarGrid.day10'
				calendarGrid.day11='calendarGrid.day11'
				calendarGrid.day12='calendarGrid.day12'
				calendarGrid.day13='calendarGrid.day13'
				calendarGrid.day14='calendarGrid.day14'
				calendarGrid.day15='calendarGrid.day15'
				calendarGrid.day16='calendarGrid.day16'
				calendarGrid.day17='calendarGrid.day17'
				calendarGrid.day18='calendarGrid.day18'
				calendarGrid.day19='calendarGrid.day19'
				calendarGrid.day20='calendarGrid.day20'
				calendarGrid.day21='calendarGrid.day21'
				calendarGrid.day22='calendarGrid.day22'
				calendarGrid.day23='calendarGrid.day23'
				calendarGrid.day24='calendarGrid.day24'
				calendarGrid.day25='calendarGrid.day25'
				calendarGrid.day26='calendarGrid.day26'
				calendarGrid.day27='calendarGrid.day27'
				calendarGrid.day28='calendarGrid.day28'
				calendarGrid.day29='calendarGrid.day29'
				calendarGrid.day30='calendarGrid.day30'
				calendarGrid.day31='calendarGrid.day31'
				calendarGrid.day32='calendarGrid.day32'
				calendarGrid.day33='calendarGrid.day33'
				calendarGrid.day34='calendarGrid.day34'
				calendarGrid.day35='calendarGrid.day35'
				calendarGrid.day36='calendarGrid.day36'
				calendarGrid.day37='calendarGrid.day37'
				calendarGrid.day38='calendarGrid.day38'
				calendarGrid.day39='calendarGrid.day39'
				calendarGrid.day40='calendarGrid.day40'
				calendarGrid.day41='calendarGrid.day41'
				calendarGrid.day42='calendarGrid.day42'
				detailPanel='detailPanel'
				detailPanel.lblDetailTitle='detailPanel.lblDetailTitle'
				detailPanel.lblDetailDesc='detailPanel.lblDetailDesc'
				detailPanel.lblDetailTime='detailPanel.lblDetailTime'
				detailPanel.lblDetailType='detailPanel.lblDetailType'
				detailPanel.imgDetailImage='detailPanel.imgDetailImage'
				detailPanel.btnDetailClose='detailPanel.btnDetailClose'
			/>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,38' Name='header' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='726,36' Size='726,36'>
				<Button LocalText='Prev' Location='8,6' Name='btnPrevMonth' ScrollExtent='50,24' Size='50,24' Style='/Styles.New.buttons.hud.style'>Prev</Button>
				<Text Enabled='false' Font='bold_16' LocalText='March 2026' Location='70,6' Name='lblMonthYear' ScrollExtent='180,24' Size='180,24' TextAlignment='Center' TextAlignmentVertical='Center' TextColor='#FFFFFF'>March 2026</Text>
				<Button LocalText='Next' Location='260,6' Name='btnNextMonth' ScrollExtent='50,24' Size='50,24' Style='/Styles.New.buttons.hud.style'>Next</Button>
				<Button LocalText='Today' Location='320,6' Name='btnToday' ScrollExtent='60,24' Size='60,24' Style='/Styles.New.buttons.hud.style'>Today</Button>
				<Button LocalText='Create Event' Location='600,6' Name='btnCreate' ScrollExtent='110,24' Size='110,24' Style='/Styles.New.buttons.hud.style'>Create Event</Button>
				<Button LocalText='Settings' Location='560,6' Name='btnSettings' ScrollExtent='80,24' Size='80,24' Style='/Styles.New.buttons.hud.style' Visible='false'>Settings</Button>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.60' BackgroundTint='#00354F' Location='12,78' Name='dayHeaders' PalBgTint='line1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='504,24' Size='504,24'>
				<Text Enabled='false' Font='bold_12' LocalText='Sun' Location='6,3' Name='lblSun' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#FF8888'>Sun</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Mon' Location='76,3' Name='lblMon' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#96F4FC'>Mon</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Tue' Location='146,3' Name='lblTue' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#96F4FC'>Tue</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Wed' Location='216,3' Name='lblWed' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#96F4FC'>Wed</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Thu' Location='286,3' Name='lblThu' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#96F4FC'>Thu</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Fri' Location='356,3' Name='lblFri' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#96F4FC'>Fri</Text>
				<Text Enabled='false' Font='bold_12' LocalText='Sat' Location='426,3' Name='lblSat' ScrollExtent='62,18' Size='62,18' TextAlignment='Center' TextColor='#8888FF'>Sat</Text>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,106' Name='calendarGrid' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='504,318' Size='504,318'>
				<Button Name='day01' Location='6,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='1'>1</Button>
				<Button Name='day02' Location='76,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='2'>2</Button>
				<Button Name='day03' Location='146,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='3'>3</Button>
				<Button Name='day04' Location='216,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='4'>4</Button>
				<Button Name='day05' Location='286,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='5'>5</Button>
				<Button Name='day06' Location='356,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='6'>6</Button>
				<Button Name='day07' Location='426,6' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='7'>7</Button>
				<Button Name='day08' Location='6,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='8'>8</Button>
				<Button Name='day09' Location='76,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='9'>9</Button>
				<Button Name='day10' Location='146,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='10'>10</Button>
				<Button Name='day11' Location='216,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='11'>11</Button>
				<Button Name='day12' Location='286,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='12'>12</Button>
				<Button Name='day13' Location='356,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='13'>13</Button>
				<Button Name='day14' Location='426,58' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='14'>14</Button>
				<Button Name='day15' Location='6,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='15'>15</Button>
				<Button Name='day16' Location='76,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='16'>16</Button>
				<Button Name='day17' Location='146,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='17'>17</Button>
				<Button Name='day18' Location='216,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='18'>18</Button>
				<Button Name='day19' Location='286,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='19'>19</Button>
				<Button Name='day20' Location='356,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='20'>20</Button>
				<Button Name='day21' Location='426,110' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='21'>21</Button>
				<Button Name='day22' Location='6,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='22'>22</Button>
				<Button Name='day23' Location='76,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='23'>23</Button>
				<Button Name='day24' Location='146,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='24'>24</Button>
				<Button Name='day25' Location='216,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='25'>25</Button>
				<Button Name='day26' Location='286,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='26'>26</Button>
				<Button Name='day27' Location='356,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='27'>27</Button>
				<Button Name='day28' Location='426,162' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='28'>28</Button>
				<Button Name='day29' Location='6,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='29'>29</Button>
				<Button Name='day30' Location='76,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='30'>30</Button>
				<Button Name='day31' Location='146,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' LocalText='31'>31</Button>
				<Button Name='day32' Location='216,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day33' Location='286,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day34' Location='356,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day35' Location='426,214' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day36' Location='6,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day37' Location='76,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day38' Location='146,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day39' Location='216,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day40' Location='286,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day41' Location='356,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
				<Button Name='day42' Location='426,266' ScrollExtent='62,46' Size='62,46' Style='/Styles.New.buttons.hud.style' Font='bold_14' Visible='false'/>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='524,78' Name='eventPanel' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='214,346' Size='214,346'>
				<Text Enabled='false' Font='bold_14' LocalText='Selected Day' Location='8,8' Name='lblSelectedDay' ScrollExtent='198,20' Size='198,20' TextAlignment='Center' TextColor='#FFFFFF'>Selected Day</Text>
				<Text Enabled='false' Font='bold_12' LocalText='March 6, 2026' Location='8,30' Name='lblSelectedDate' ScrollExtent='198,18' Size='198,18' TextAlignment='Center' TextColor='#FFFFFF'>March 6, 2026</Text>
				<Text Enabled='false' Font='bold_11' LocalText='Events:' Location='8,56' Name='lblEventsHeader' ScrollExtent='198,16' Size='198,16' TextColor='#96F4FC'>Events:</Text>
				<DataSource Name='dataEvents' Size='128,64'/>
				<List BackgroundColor='#000000' BackgroundOpacity='0.40' DataSource='dataEvents' Location='8,76' Name='lstEvents' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='198,220' SelectionAllowedMultiRow='false' Size='198,220' Style='/Styles.list.combo.style'/>
				<Button LocalText='View Details' Location='8,306' Name='btnViewDetails' ScrollExtent='96,28' Size='96,28' Style='/Styles.New.buttons.hud.style'>View Details</Button>
				<Button LocalText='Delete' Location='112,306' Name='btnDeleteEvent' ScrollExtent='94,28' Size='94,28' Style='/Styles.New.buttons.hud.style' Visible='false'>Delete</Button>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='524,78' Name='detailPanel' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='214,346' Size='214,346' Visible='false' GetsInput='false'>
				<Text Color='#FFFFFF' Font='bold_14' LocalText='Event Title' Location='8,8' Name='lblDetailTitle' PalText='header' ScrollExtent='198,20' Size='198,20' TextAlignment='Center'>Event Title</Text>
				<Text Color='#96F4FC' Font='bold_11' LocalText='Type:' Location='8,32' Name='lblDetailType' PalText='header' ScrollExtent='198,16' Size='198,16'>Type:</Text>
				<Text Color='#96F4FC' Font='bold_11' LocalText='Time:' Location='8,50' Name='lblDetailTime' PalText='header' ScrollExtent='198,16' Size='198,16'>Time:</Text>
				<Text Color='#FFFFFF' Font='bold_11' LocalText='No description.' Location='8,72' Name='lblDetailDesc' ScrollExtent='198,120' Size='198,120' TextColor='#CCCCCC' Wrap='true'>No description.</Text>
				<Image Location='8,198' Name='imgDetailImage' ScrollExtent='198,110' Size='198,110'/>
				<Button LocalText='Close' Location='60,314' Name='btnDetailClose' ScrollExtent='96,24' Size='96,24' Style='/Styles.New.buttons.hud.style'>Close</Button>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,430' Name='legend' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='726,30' Size='726,30'>
				<Page BackgroundColor='#FFD700' BackgroundOpacity='1.00' Location='12,9' Name='staffColor' ScrollExtent='12,12' Size='12,12'/>
				<Text Font='bold_12' LocalText='[S] Staff' Location='28,5' Name='lblStaff' ScrollExtent='80,20' Size='80,20' TextColor='#FFD700'>[S] Staff</Text>
				<Page BackgroundColor='#FF4500' BackgroundOpacity='1.00' Location='120,9' Name='serverColor' ScrollExtent='12,12' Size='12,12'/>
				<Text Font='bold_12' LocalText='[E] Server' Location='136,5' Name='lblServer' ScrollExtent='90,20' Size='90,20' TextColor='#FF4500'>[E] Server</Text>
				<Page BackgroundColor='#00FF00' BackgroundOpacity='1.00' Location='240,9' Name='guildColor' ScrollExtent='12,12' Size='12,12'/>
				<Text Font='bold_12' LocalText='[G] Guild' Location='256,5' Name='lblGuild' ScrollExtent='80,20' Size='80,20' TextColor='#00FF00'>[G] Guild</Text>
				<Page BackgroundColor='#00BFFF' BackgroundOpacity='1.00' Location='350,9' Name='cityColor' ScrollExtent='12,12' Size='12,12'/>
				<Text Font='bold_12' LocalText='[C] City' Location='366,5' Name='lblCity' ScrollExtent='70,20' Size='70,20' TextColor='#00BFFF'>[C] City</Text>
			</Page>
			<Button IsCancelButton='true' LocalText='Close' Location='638,470' Name='btnClose' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Close</Button>
			<Page Name='bg' PackLocation='nfn,nfn' PackSize='a,a' ScrollExtent='750,550' Size='750,550'>
				<Page Location='738,8' Name='mmc' PackLocation='fff,nfn' PackSize='f,f' RStyleDefault='/Styles.New.thin_mmc.rs_default' ScrollExtent='12,10' Size='12,10'>
					<Button BackgroundColor='#FFFFFF' BackgroundOpacity='0.80' IsCancelButton='true' Location='-3,-3' Name='close' PackLocation='fff,cfc' PackSize='f,f' ScrollExtent='16,16' Size='16,16' Style='/Styles.Frame.buttonClose.style'/>
				</Page>
				<Page MinimumSize='0,19' Name='caption' PackLocation='nfn,nfn' PackSize='a,f' ScrollExtent='550,19' Size='550,19'>
					<Text Enabled='false' Font='bold_13' LocalText='GALACTIC CALENDAR' Margin='6,-2,0,0' MaxLines='1' MinimumSize='0,19' Name='lblTitle' PackSize='a,f' PalText='titletext' ScrollExtent='550,19' Size='550,19' TextAlignmentVertical='Center' TextCapital='true' TextColor='#00354F'>GALACTIC CALENDAR</Text>
				</Page>
				<Page BackgroundTint='#00D6FB' GetsInput='false' Location='1,0' Name='frame' PackLocation='nfn,nfn' PackSize='a,a' PalBgTint='baselight' RStyleDefault='/Styles.New.thin_in.rs_default' ScrollExtent='749,550' Selectable='true' Size='749,550'>
					<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='1,0' Name='lines' PackSize='a,a' RStyleDefault='/Styles.space.boxes.generic4.rs_default' ScrollExtent='749,550' Size='749,550'>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='0,528' Name='outlineBL' OpacityRelativeMin='0.80' PackLocation='nfn,fff' PackSize='p,f' ScrollExtent='484,22' Size='484,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomLeft2.rs_default' ScrollExtent='484,9' Size='484,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='265,528' Name='outlineBR' OpacityRelativeMin='0.80' PackLocation='fff,fff' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='484,22' Size='484,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomRight2.rs_default' ScrollExtent='484,9' Size='484,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='265,0' Name='outlineTR' OpacityRelativeMin='0.80' PackLocation='fff,nfn' PackSize='p,f' ScrollExtent='484,22' Size='484,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopRight2.rs_default' ScrollExtent='484,9' Size='484,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Name='outlineTL' OpacityRelativeMin='0.80' PackLocation='nfn,nfn' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='484,22' Size='484,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopLeft2.rs_default' ScrollExtent='484,9' Size='484,9'/>
						</Page>
					</Page>
					<Page BackgroundOpacity='1.00' Location='-1,0' Name='top' PackLocation='nfn,nfn' PackSize='a,f' RStyleDefault='/Styles.New.main_TR.rs_default' ScrollExtent='750,29' Size='750,29'/>
					<Page BackgroundOpacity='1.00' Location='0,527' Name='bottom' PackLocation='nfn,fff' PackSize='a,f' RStyleDefault='/Styles.New.main_BR.rs_default' ScrollExtent='749,23' Size='749,23'/>
				</Page>
			</Page>
		</Page>
	</Page>
	<Page
		BackgroundColor='#000000'
		BackgroundOpacity='0.70'
		Name='CalendarEventEditor'
		PackSize='1,1'
		ScrollExtent='1024,768'
		Size='1024,768'
		Visible='false'
		VisualEditLock='true'
	>
		<Page
			BackgroundColor='#404040'
			BackgroundOpacity='0.95'
			BackgroundTint='#00354F'
			Location='262,100'
			MinimumSize='500,500'
			Name='eventEditor'
			PalBgTint='back1'
			ScrollExtent='500,500'
			Size='500,500'
			UserMovable='true'
			UserResizable='false'
		>
			<Data
				Name='CodeData'
				titleSection.txtTitle='titleSection.txtTitle'
				descSection.txtDescription='descSection.txtDescription'
				typeSection.cmbEventType='typeSection.cmbEventType'
				dateSection.txtDate='dateSection.txtDate'
				dateSection.txtTime='dateSection.txtTime'
				durationSection.cmbDuration='durationSection.cmbDuration'
				serverEventSection='serverEventSection'
				serverEventSection.cmbServerEvent='serverEventSection.cmbServerEvent'
				optionsSection.chkBroadcast='optionsSection.chkBroadcast'
				optionsSection.chkRecurring='optionsSection.chkRecurring'
				recurrenceSection='recurrenceSection'
				recurrenceSection.cmbRecurrence='recurrenceSection.cmbRecurrence'
				imageSection.cmbImage='imageSection.cmbImage'
				btnCreateEvent='btnCreateEvent'
				btnCancel='btnCancel'
			/>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,38' Name='titleSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='476,70' Size='476,70'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Event Title:' Location='8,6' Name='lblTitle' PalText='header' ScrollExtent='120,18' Size='120,18'>Event Title:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.70' Cursor='.' EditableUnicode='true' Font='bold_14' Location='8,30' MaxLines='1' Name='txtTitle' PalBgTint='line1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='460,34' Size='460,34' Style='/Styles.textbox.default.Style_12' TextColor='#FFFFFF'/>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,114' Name='descSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='476,76' Size='476,76'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Description:' Location='8,6' Name='lblDesc' PalText='header' ScrollExtent='120,18' Size='120,18'>Description:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.70' Cursor='.' EditableUnicode='true' Font='bold_14' Location='8,28' MaxLines='3' Name='txtDescription' PalBgTint='line1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='460,42' Size='460,42' Style='/Styles.textbox.default.Style_12' TextColor='#FFFFFF'/>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,196' Name='typeSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,68' Size='230,68'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Event Type:' Location='8,6' Name='lblType' PalText='header' ScrollExtent='100,18' Size='100,18'>Event Type:</Text>
				<ComboBox BackgroundOpacity='0.70' ButtonMargin='2,2,2,0' ButtonStyle='/Styles.New.buttons.combo_arrow.style' ButtonWidth='10' DataSource='dsEventType' DropDownHeight='128' ListStyle='/Styles.list.combo.style_all' Location='8,32' MinimumSize='0,22' Name='cmbEventType' RStyleDefault='/Styles.New.combo.combo1.rs_default' ScrollbarStyle='/Styles.scrollbar.default.vHud3.style' ScrollbarWidth='11' ScrollExtent='214,22' SelectedIndex='0' Size='214,22' TextboxMargin='0,1,0,0' TextboxStyle='/Styles.textbox.combo.Style_new'/>
				<DataSource Name='dsEventType'>
					<Data Name='0' Text='Staff Event'/>
					<Data Name='1' Text='Guild Event'/>
					<Data Name='2' Text='City Event'/>
					<Data Name='3' Text='Server Event'/>
				</DataSource>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='258,196' Name='dateSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,76' Size='230,76'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Date / Time:' Location='8,6' Name='lblDate' PalText='header' ScrollExtent='100,18' Size='100,18'>Date / Time:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.70' Cursor='.' EditableUnicode='true' Font='bold_14' LocalText='2026-03-06' Location='8,32' MaxLength='10' MaxLines='1' Name='txtDate' PalBgTint='line1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='120,34' Size='120,34' Style='/Styles.textbox.default.Style_12' TextColor='#FFFFFF'>2026-03-06</Textbox>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.70' Cursor='.' EditableUnicode='true' Font='bold_14' LocalText='12:00' Location='136,32' MaxLength='5' MaxLines='1' Name='txtTime' PalBgTint='line1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='86,34' Size='86,34' Style='/Styles.textbox.default.Style_12' TextColor='#FFFFFF'>12:00</Textbox>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,278' Name='durationSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,68' Size='230,68'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Duration:' Location='8,6' Name='lblDuration' PalText='header' ScrollExtent='100,18' Size='100,18'>Duration:</Text>
				<ComboBox BackgroundOpacity='0.70' ButtonMargin='2,2,2,0' ButtonStyle='/Styles.New.buttons.combo_arrow.style' ButtonWidth='10' DataSource='dsDuration' DropDownHeight='180' ListStyle='/Styles.list.combo.style_all' Location='8,32' MinimumSize='0,22' Name='cmbDuration' RStyleDefault='/Styles.New.combo.combo1.rs_default' ScrollbarStyle='/Styles.scrollbar.default.vHud3.style' ScrollbarWidth='11' ScrollExtent='214,22' SelectedIndex='2' Size='214,22' TextboxMargin='0,1,0,0' TextboxStyle='/Styles.textbox.combo.Style_new'/>
				<DataSource Name='dsDuration'>
					<Data Name='0' Text='15 minutes'/>
					<Data Name='1' Text='30 minutes'/>
					<Data Name='2' Text='1 hour'/>
					<Data Name='3' Text='2 hours'/>
					<Data Name='4' Text='4 hours'/>
					<Data Name='5' Text='8 hours'/>
					<Data Name='6' Text='All day'/>
				</DataSource>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='258,278' Name='serverEventSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,68' Size='230,68' Visible='false'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Server Event:' Location='8,6' Name='lblServerEvent' PalText='header' ScrollExtent='120,18' Size='120,18'>Server Event:</Text>
				<ComboBox BackgroundOpacity='0.70' ButtonMargin='2,2,2,0' ButtonStyle='/Styles.New.buttons.combo_arrow.style' ButtonWidth='10' DataSource='dsServerEvent' DropDownHeight='128' ListStyle='/Styles.list.combo.style_all' Location='8,32' MinimumSize='0,22' Name='cmbServerEvent' RStyleDefault='/Styles.New.combo.combo1.rs_default' ScrollbarStyle='/Styles.scrollbar.default.vHud3.style' ScrollbarWidth='11' ScrollExtent='214,22' SelectedIndex='0' Size='214,22' TextboxMargin='0,1,0,0' TextboxStyle='/Styles.textbox.combo.Style_new'/>
				<DataSource Name='dsServerEvent'>
					<Data Name='0' Text='Halloween'/>
					<Data Name='1' Text='Life Day'/>
					<Data Name='2' Text='Love Day'/>
					<Data Name='3' Text='Empire Day Ceremony'/>
				</DataSource>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,352' Name='optionsSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='476,50' Size='476,50'>
				<Checkbox Color='#96F4FC' LocalText='Broadcast when event starts' Location='8,14' MinimumSize='0,22' Name='chkBroadcast' ScrollExtent='220,22' Size='220,22' Style='/Styles.New.Checkbox2.Style'>Broadcast when event starts</Checkbox>
				<Checkbox Color='#96F4FC' LocalText='Recurring event' Location='260,14' MinimumSize='0,22' Name='chkRecurring' ScrollExtent='150,22' Size='150,22' Style='/Styles.New.Checkbox2.Style'>Recurring event</Checkbox>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,408' Name='recurrenceSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,68' Size='230,68' Visible='false'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Repeat:' Location='8,6' Name='lblRecurrence' PalText='header' ScrollExtent='100,18' Size='100,18'>Repeat:</Text>
				<ComboBox BackgroundOpacity='0.70' ButtonMargin='2,2,2,0' ButtonStyle='/Styles.New.buttons.combo_arrow.style' ButtonWidth='10' DataSource='dsRecurrence' DropDownHeight='128' ListStyle='/Styles.list.combo.style_all' Location='8,32' MinimumSize='0,22' Name='cmbRecurrence' RStyleDefault='/Styles.New.combo.combo1.rs_default' ScrollbarStyle='/Styles.scrollbar.default.vHud3.style' ScrollbarWidth='11' ScrollExtent='214,22' SelectedIndex='0' Size='214,22' TextboxMargin='0,1,0,0' TextboxStyle='/Styles.textbox.combo.Style_new'/>
				<DataSource Name='dsRecurrence'>
					<Data Name='0' Text='Daily'/>
					<Data Name='1' Text='Weekly'/>
					<Data Name='2' Text='Monthly'/>
					<Data Name='3' Text='Yearly'/>
				</DataSource>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='258,408' Name='imageSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='230,68' Size='230,68'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Event Image:' Location='8,6' Name='lblImage' PalText='header' ScrollExtent='120,18' Size='120,18'>Event Image:</Text>
				<ComboBox BackgroundOpacity='0.70' ButtonMargin='2,2,2,0' ButtonStyle='/Styles.New.buttons.combo_arrow.style' ButtonWidth='10' DropDownHeight='400' ListStyle='/Styles.list.combo.style_all' Location='8,32' MinimumSize='0,22' Name='cmbImage' RStyleDefault='/Styles.New.combo.combo1.rs_default' ScrollbarStyle='/Styles.scrollbar.default.vHud3.style' ScrollbarWidth='11' ScrollExtent='214,22' SelectedIndex='0' Size='214,22' TextboxMargin='0,1,0,0' TextboxStyle='/Styles.textbox.combo.Style_new'/>
			</Page>
			<Button LocalText='Create Event' Location='280,482' Name='btnCreateEvent' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Create Event</Button>
			<Button IsCancelButton='true' LocalText='Cancel' Location='388,482' Name='btnCancel' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Cancel</Button>
			<Page Name='bg' PackLocation='nfn,nfn' PackSize='a,a' ScrollExtent='500,500' Size='500,500'>
				<Page Location='488,8' Name='mmc' PackLocation='fff,nfn' PackSize='f,f' RStyleDefault='/Styles.New.thin_mmc.rs_default' ScrollExtent='12,10' Size='12,10'>
					<Button BackgroundColor='#FFFFFF' BackgroundOpacity='0.80' IsCancelButton='true' Location='-3,-3' Name='close' PackLocation='fff,cfc' PackSize='f,f' ScrollExtent='16,16' Size='16,16' Style='/Styles.Frame.buttonClose.style'/>
				</Page>
				<Page MinimumSize='0,19' Name='caption' PackLocation='nfn,nfn' PackSize='a,f' ScrollExtent='400,19' Size='400,19'>
					<Text Enabled='false' Font='bold_13' LocalText='CREATE EVENT' Margin='6,-2,0,0' MaxLines='1' MinimumSize='0,19' Name='lblTitle' PackSize='a,f' PalText='titletext' ScrollExtent='400,19' Size='400,19' TextAlignmentVertical='Center' TextCapital='true' TextColor='#00354F'>CREATE EVENT</Text>
				</Page>
				<Page BackgroundTint='#00D6FB' GetsInput='false' Location='1,0' Name='frame' PackLocation='nfn,nfn' PackSize='a,a' PalBgTint='baselight' RStyleDefault='/Styles.New.thin_in.rs_default' ScrollExtent='499,500' Selectable='true' Size='499,500'>
					<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='1,0' Name='lines' PackSize='a,a' RStyleDefault='/Styles.space.boxes.generic4.rs_default' ScrollExtent='499,500' Size='499,500'>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='0,478' Name='outlineBL' OpacityRelativeMin='0.80' PackLocation='nfn,fff' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,478' Name='outlineBR' OpacityRelativeMin='0.80' PackLocation='fff,fff' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,0' Name='outlineTR' OpacityRelativeMin='0.80' PackLocation='fff,nfn' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Name='outlineTL' OpacityRelativeMin='0.80' PackLocation='nfn,nfn' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
					</Page>
					<Page BackgroundOpacity='1.00' Location='-1,0' Name='top' PackLocation='nfn,nfn' PackSize='a,f' RStyleDefault='/Styles.New.main_TR.rs_default' ScrollExtent='500,29' Size='500,29'/>
					<Page BackgroundOpacity='1.00' Location='0,477' Name='bottom' PackLocation='nfn,fff' PackSize='a,f' RStyleDefault='/Styles.New.main_BR.rs_default' ScrollExtent='499,23' Size='499,23'/>
				</Page>
			</Page>
		</Page>
	</Page>
	<Page
		BackgroundColor='#000000'
		BackgroundOpacity='0.70'
		Name='CalendarSettings'
		PackSize='1,1'
		ScrollExtent='1024,768'
		Size='1024,768'
		Visible='false'
		VisualEditLock='true'
	>
		<Page
			BackgroundColor='#404040'
			BackgroundOpacity='0.95'
			BackgroundTint='#00354F'
			Location='312,184'
			MinimumSize='400,360'
			Name='settingsWindow'
			PalBgTint='back1'
			ScrollExtent='400,360'
			Size='400,360'
			UserMovable='true'
			UserResizable='false'
		>
			<Data
				Name='CodeData'
				textureSection.txtTexture='textureSection.txtTexture'
				rectSection.txtRectX='rectSection.txtRectX'
				rectSection.txtRectY='rectSection.txtRectY'
				rectSection.txtRectW='rectSection.txtRectW'
				rectSection.txtRectH='rectSection.txtRectH'
				rectSection.btnPreview='rectSection.btnPreview'
				previewSection.previewPane.imgPreview='previewSection.previewPane.imgPreview'
				btnApply='btnApply'
				btnReset='btnReset'
				btnClose='btnClose'
			/>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,38' Name='textureSection' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='376,58' Size='376,58'>
				<Text Enabled='false' Font='bold_12' LocalText='Background Texture:' Location='8,6' Name='lblTexture' ScrollExtent='150,18' Size='150,18' TextColor='#96F4FC'>Background Texture:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.40' Cursor='.' EditableUnicode='true' Location='8,26' MaxLines='1' Name='txtTexture' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='360,26' Size='360,26' Style='/Styles.textbox.default.style' TextColor='#FFFFFF'/>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,102' Name='rectSection' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='376,76' Size='376,76'>
				<Text Enabled='false' Font='bold_12' LocalText='Source Rectangle:' Location='8,6' Name='lblRect' ScrollExtent='150,18' Size='150,18' TextColor='#96F4FC'>Source Rectangle:</Text>
				<Text Enabled='false' Font='bold_10' LocalText='X:' Location='8,30' Name='lblX' ScrollExtent='16,16' Size='16,16' TextColor='#FFFFFF'>X:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.40' Cursor='.' EditableUnicode='true' Location='24,28' MaxLength='5' MaxLines='1' Name='txtRectX' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='60,22' Size='60,22' Style='/Styles.textbox.default.style' TextColor='#FFFFFF'>0</Textbox>
				<Text Enabled='false' Font='bold_10' LocalText='Y:' Location='94,30' Name='lblY' ScrollExtent='16,16' Size='16,16' TextColor='#FFFFFF'>Y:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.40' Cursor='.' EditableUnicode='true' Location='110,28' MaxLength='5' MaxLines='1' Name='txtRectY' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='60,22' Size='60,22' Style='/Styles.textbox.default.style' TextColor='#FFFFFF'>0</Textbox>
				<Text Enabled='false' Font='bold_10' LocalText='W:' Location='180,30' Name='lblW' ScrollExtent='20,16' Size='20,16' TextColor='#FFFFFF'>W:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.40' Cursor='.' EditableUnicode='true' Location='200,28' MaxLength='5' MaxLines='1' Name='txtRectW' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='60,22' Size='60,22' Style='/Styles.textbox.default.style' TextColor='#FFFFFF'>512</Textbox>
				<Text Enabled='false' Font='bold_10' LocalText='H:' Location='270,30' Name='lblH' ScrollExtent='16,16' Size='16,16' TextColor='#FFFFFF'>H:</Text>
				<Textbox BackgroundColor='#000000' BackgroundOpacity='0.40' Cursor='.' EditableUnicode='true' Location='286,28' MaxLength='5' MaxLines='1' Name='txtRectH' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='60,22' Size='60,22' Style='/Styles.textbox.default.style' TextColor='#FFFFFF'>512</Textbox>
				<Button LocalText='Preview' Location='8,52' Name='btnPreview' ScrollExtent='80,20' Size='80,20' Style='/Styles.New.buttons.hud.style'>Preview</Button>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,184' Name='previewSection' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='376,120' Size='376,120'>
				<Text Enabled='false' Font='bold_12' LocalText='Preview:' Location='8,6' Name='lblPreview' ScrollExtent='100,18' Size='100,18' TextColor='#96F4FC'>Preview:</Text>
				<Page BackgroundColor='#000000' BackgroundOpacity='0.60' Location='8,26' Name='previewPane' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='360,86' Size='360,86'>
					<Image Location='4,4' Name='imgPreview' ScrollExtent='352,78' Size='352,78'/>
				</Page>
			</Page>
			<Button LocalText='Apply' Location='180,312' Name='btnApply' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Apply</Button>
			<Button LocalText='Reset' Location='68,312' Name='btnReset' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Reset</Button>
			<Button IsCancelButton='true' LocalText='Close' Location='288,312' Name='btnClose' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Close</Button>
			<Page Name='bg' PackLocation='nfn,nfn' PackSize='a,a' ScrollExtent='400,360' Size='400,360'>
				<Page Location='388,8' Name='mmc' PackLocation='fff,nfn' PackSize='f,f' RStyleDefault='/Styles.New.thin_mmc.rs_default' ScrollExtent='12,10' Size='12,10'>
					<Button BackgroundColor='#FFFFFF' BackgroundOpacity='0.80' IsCancelButton='true' Location='-3,-3' Name='close' PackLocation='fff,cfc' PackSize='f,f' ScrollExtent='16,16' Size='16,16' Style='/Styles.Frame.buttonClose.style'/>
				</Page>
				<Page MinimumSize='0,19' Name='caption' PackLocation='nfn,nfn' PackSize='a,f' ScrollExtent='300,19' Size='300,19'>
					<Text Enabled='false' Font='bold_13' LocalText='CALENDAR SETTINGS' Margin='6,-2,0,0' MaxLines='1' MinimumSize='0,19' Name='lblTitle' PackSize='a,f' PalText='titletext' ScrollExtent='300,19' Size='300,19' TextAlignmentVertical='Center' TextCapital='true' TextColor='#00354F'>CALENDAR SETTINGS</Text>
				</Page>
				<Page BackgroundTint='#00D6FB' GetsInput='false' Location='1,0' Name='frame' PackLocation='nfn,nfn' PackSize='a,a' PalBgTint='baselight' RStyleDefault='/Styles.New.thin_in.rs_default' ScrollExtent='399,360' Selectable='true' Size='399,360'>
					<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='1,0' Name='lines' PackSize='a,a' RStyleDefault='/Styles.space.boxes.generic4.rs_default' ScrollExtent='399,360' Size='399,360'>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='0,338' Name='outlineBL' OpacityRelativeMin='0.80' PackLocation='nfn,fff' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,338' Name='outlineBR' OpacityRelativeMin='0.80' PackLocation='fff,fff' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,0' Name='outlineTR' OpacityRelativeMin='0.80' PackLocation='fff,nfn' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Name='outlineTL' OpacityRelativeMin='0.80' PackLocation='nfn,nfn' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
					</Page>
					<Page BackgroundOpacity='1.00' Location='-1,0' Name='top' PackLocation='nfn,nfn' PackSize='a,f' RStyleDefault='/Styles.New.main_TR.rs_default' ScrollExtent='400,29' Size='400,29'/>
					<Page BackgroundOpacity='1.00' Location='0,337' Name='bottom' PackLocation='nfn,fff' PackSize='a,f' RStyleDefault='/Styles.New.main_BR.rs_default' ScrollExtent='399,23' Size='399,23'/>
				</Page>
			</Page>
		</Page>
	</Page>
	<Page
		BackgroundColor='#000000'
		BackgroundOpacity='0.70'
		Name='CalendarEventDetails'
		PackSize='1,1'
		ScrollExtent='1024,768'
		Size='1024,768'
		Visible='false'
		VisualEditLock='true'
	>
		<Page
			BackgroundColor='#404040'
			BackgroundOpacity='0.95'
			BackgroundTint='#00354F'
			Location='312,150'
			MinimumSize='400,400'
			Name='detailsWindow'
			PalBgTint='back1'
			ScrollExtent='400,400'
			Size='400,400'
			UserMovable='true'
			UserResizable='false'
		>
			<Data
				Name='CodeData'
				lblTitle='lblTitle'
				lblType='lblType'
				lblTime='lblTime'
				lblDate='lblDate'
				lblDescription='lblDescription'
				imgEvent='imgEvent'
				btnClose='btnClose'
				btnEdit='btnEdit'
				btnDelete='btnDelete'
			/>
			<Text Color='#FFFFFF' Font='bold_16' LocalText='Event Title' Location='12,38' Name='lblTitle' ScrollExtent='376,24' Size='376,24' TextAlignment='Center'>Event Title</Text>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,70' Name='infoSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='376,70' Size='376,70'>
				<Text Color='#FFD700' Font='bold_12' LocalText='Type: Staff Event' Location='8,8' Name='lblType' ScrollExtent='360,18' Size='360,18'>Type: Staff Event</Text>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Date: March 7, 2026' Location='8,28' Name='lblDate' ScrollExtent='360,18' Size='360,18'>Date: March 7, 2026</Text>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Time: 12:00 - Duration: 1h' Location='8,48' Name='lblTime' ScrollExtent='360,18' Size='360,18'>Time: 12:00 - Duration: 1h</Text>
			</Page>
			<Page BackgroundColor='#FFFFFF' BackgroundOpacity='0.50' BackgroundTint='#00354F' Location='12,146' Name='descSection' PalBgTint='back1' RStyleDefault='/Styles.New.whiteOutline.rs_default' ScrollExtent='376,100' Size='376,100'>
				<Text Color='#96F4FC' Font='bold_12' LocalText='Description:' Location='8,6' Name='lblDescHeader' ScrollExtent='100,18' Size='100,18'>Description:</Text>
				<Text Color='#FFFFFF' Font='bold_11' LocalText='No description provided.' Location='8,26' Name='lblDescription' ScrollExtent='360,68' Size='360,68' Wrap='true'>No description provided.</Text>
			</Page>
			<Page BackgroundColor='#000000' BackgroundOpacity='0.60' Location='12,252' Name='imageSection' RStyleDefault='/Styles.New.blackbox.rs_default' ScrollExtent='376,100' Size='376,100'>
				<Image Location='4,4' Name='imgEvent' ScrollExtent='368,92' Size='368,92'/>
			</Page>
			<Button LocalText='Close' Location='150,360' Name='btnClose' ScrollExtent='100,32' Size='100,32' Style='/Styles.New.buttons.hud.style'>Close</Button>
			<Button LocalText='Edit' Location='12,360' Name='btnEdit' ScrollExtent='80,32' Size='80,32' Style='/Styles.New.buttons.hud.style' Visible='false'>Edit</Button>
			<Button LocalText='Delete' Location='308,360' Name='btnDelete' ScrollExtent='80,32' Size='80,32' Style='/Styles.New.buttons.hud.style' Visible='false'>Delete</Button>
			<Page Name='bg' PackLocation='nfn,nfn' PackSize='a,a' ScrollExtent='400,400' Size='400,400'>
				<Page Location='388,8' Name='mmc' PackLocation='fff,nfn' PackSize='f,f' RStyleDefault='/Styles.New.thin_mmc.rs_default' ScrollExtent='12,10' Size='12,10'>
					<Button BackgroundColor='#FFFFFF' BackgroundOpacity='0.80' IsCancelButton='true' Location='-3,-3' Name='close' PackLocation='fff,cfc' PackSize='f,f' ScrollExtent='16,16' Size='16,16' Style='/Styles.Frame.buttonClose.style'/>
				</Page>
				<Page MinimumSize='0,19' Name='caption' PackLocation='nfn,nfn' PackSize='a,f' ScrollExtent='300,19' Size='300,19'>
					<Text Enabled='false' Font='bold_13' LocalText='EVENT DETAILS' Margin='6,-2,0,0' MaxLines='1' MinimumSize='0,19' Name='lblCaption' PackSize='a,f' PalText='titletext' ScrollExtent='300,19' Size='300,19' TextAlignmentVertical='Center' TextCapital='true' TextColor='#00354F'>EVENT DETAILS</Text>
				</Page>
				<Page BackgroundTint='#00D6FB' GetsInput='false' Location='1,0' Name='frame' PackLocation='nfn,nfn' PackSize='a,a' PalBgTint='baselight' RStyleDefault='/Styles.New.thin_in.rs_default' ScrollExtent='399,400' Selectable='true' Size='399,400'>
					<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='1,0' Name='lines' PackSize='a,a' RStyleDefault='/Styles.space.boxes.generic4.rs_default' ScrollExtent='399,400' Size='399,400'>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='0,378' Name='outlineBL' OpacityRelativeMin='0.80' PackLocation='nfn,fff' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,378' Name='outlineBR' OpacityRelativeMin='0.80' PackLocation='fff,fff' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Location='0,13' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlinebottomRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Location='179,0' Name='outlineTR' OpacityRelativeMin='0.80' PackLocation='fff,nfn' PackSize='p,f' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopRight2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
						<Page BackgroundColor='#FFFFFF' GetsInput='false' Name='outlineTL' OpacityRelativeMin='0.80' PackLocation='nfn,nfn' PackSize='p,f' RStyleDefault='/Styles.space.boxes.boxlineTopLeft.rs_default' ScrollExtent='220,22' Size='220,22'>
							<Page BackgroundColor='#FFFFFF' BackgroundOpacity='1.00' BackgroundTint='#1CFFFF' GetsInput='false' Name='outline1' OpacityRelativeMin='0.80' PackSize='a,a' PalBgTint='line1' RStyleDefault='/Styles.space.boxes.boxlineTopLeft2.rs_default' ScrollExtent='220,9' Size='220,9'/>
						</Page>
					</Page>
					<Page BackgroundOpacity='1.00' Location='-1,0' Name='top' PackLocation='nfn,nfn' PackSize='a,f' RStyleDefault='/Styles.New.main_TR.rs_default' ScrollExtent='400,29' Size='400,29'/>
					<Page BackgroundOpacity='1.00' Location='0,377' Name='bottom' PackLocation='nfn,fff' PackSize='a,f' RStyleDefault='/Styles.New.main_BR.rs_default' ScrollExtent='399,23' Size='399,23'/>
				</Page>
			</Page>
		</Page>
	</Page>

